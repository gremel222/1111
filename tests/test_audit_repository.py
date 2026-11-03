"""Tests for audit tooling only; these are not Unreal/gameplay tests."""

from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from tools import audit_repository as audit


class InventoryTests(unittest.TestCase):
    def test_empty_inventory_has_no_unreal_project(self):
        result = audit.build_inventory([])
        self.assertEqual(result["tracked_file_count"], 0)
        self.assertEqual(result["unreal_projects"], [])
        self.assertFalse(any(result["project_directories"].values()))

    def test_detects_unreal_artifacts_without_claiming_they_work(self):
        result = audit.build_inventory([
            "Game.uproject", "Source/Game/Game.Build.cs", "Source/GameEditor.Target.cs",
            "Source/Game/Runtime.cpp", "Source/Game/Runtime.h", "Plugins/Tool/Tool.uplugin",
            "Content/Hero.uasset", "Content/Arena.umap", "Docs/Notes.md",
        ])
        self.assertEqual(result["unreal_projects"], ["Game.uproject"])
        self.assertEqual(result["unreal_plugins"], ["Plugins/Tool/Tool.uplugin"])
        self.assertEqual(len(result["unreal_assets"]), 2)
        self.assertEqual(len(result["unreal_build_descriptors"]), 2)
        self.assertEqual(len(result["cpp_files"]), 2)
        self.assertEqual(len(result["csharp_files"]), 2)
        self.assertTrue(result["project_directories"]["Source"])
        self.assertNotIn("implementation_checks", result)

    def test_inventory_is_deduplicated_and_sorted(self):
        result = audit.build_inventory(["b.md", "a.md", "b.md"])
        self.assertEqual(result["tracked_files"], ["a.md", "b.md"])
        self.assertEqual(result["extensions"], {".md": 2})

    def test_python_tests_are_only_tooling_test_candidates(self):
        result = audit.build_inventory(["tests/test_audit.py", "tools/audit.py"])
        self.assertEqual(result["python_test_candidates"], ["tests/test_audit.py"])


class MarkdownLinkTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        (self.root / "Docs").mkdir()

    def write(self, name, text):
        (self.root / name).write_text(text, encoding="utf-8")

    def test_relative_link_and_fragment_resolve(self):
        self.write("README.md", "[Passport](Docs/01_GAME_PASSPORT.md#section)")
        self.write("Docs/01_GAME_PASSPORT.md", "# Passport")
        result = audit.check_markdown_links(self.root, ["README.md", "Docs/01_GAME_PASSPORT.md"])
        self.assertEqual(result, {"checked_local_links": 1, "issues": []})

    def test_missing_link_is_reported_with_line_number(self):
        self.write("README.md", "# Title\n[Missing](Docs/missing.md)")
        result = audit.check_markdown_links(self.root, ["README.md"])
        self.assertEqual(result["issues"], [{
            "file": "README.md", "line": 2, "target": "Docs/missing.md", "status": "MISSING",
        }])

    def test_external_and_anchor_links_are_not_file_checks(self):
        self.write("README.md", "[Web](https://example.com/a) [Mail](mailto:a@example.com) [Here](#title)")
        result = audit.check_markdown_links(self.root, ["README.md"])
        self.assertEqual(result["checked_local_links"], 0)
        self.assertEqual(result["issues"], [])

    def test_fenced_examples_are_ignored(self):
        self.write("README.md", "# Example\n```markdown\n[Example](missing.md)\n```\n~~~\n[Other](other.md)\n~~~")
        result = audit.check_markdown_links(self.root, ["README.md"])
        self.assertEqual(result["issues"], [])

    def test_escaping_link_is_reported(self):
        self.write("Docs/README.md", "[Outside](../../outside.txt)")
        result = audit.check_markdown_links(self.root, ["Docs/README.md"])
        self.assertEqual(result["issues"][0]["status"], "OUTSIDE_REPOSITORY")

    def test_missing_tracked_markdown_is_reported(self):
        result = audit.check_markdown_links(self.root, ["Docs/missing.md"])
        self.assertEqual(result["issues"][0]["status"], "MISSING_FILE")

    def test_encoded_spaces_and_title_are_supported(self):
        self.write("Docs/My Passport.md", "# Passport")
        self.write("README.md", '[Passport](Docs/My%20Passport.md "Title")')
        result = audit.check_markdown_links(self.root, ["README.md", "Docs/My Passport.md"])
        self.assertEqual(result["issues"], [])
        self.assertEqual(result["checked_local_links"], 1)


class CollectAuditTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        for name in audit.REQUIRED_DOCUMENTS:
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text("# Project memory\n", encoding="utf-8")

        def fake_git(root, *arguments):
            self.assertEqual(root, self.root)
            responses = {
                ("ls-files", "-z"): "\0".join(audit.REQUIRED_DOCUMENTS) + "\0",
                ("branch", "--show-current"): "test-branch\n",
                ("rev-parse", "HEAD"): "0123456789abcdef\n",
                ("status", "--porcelain"): "",
            }
            return responses[arguments]

        patcher = patch.object(audit, "git", side_effect=fake_git)
        patcher.start()
        self.addCleanup(patcher.stop)

    def test_missing_ue_and_editor_do_not_mean_the_inventory_failed(self):
        report = audit.collect_audit(
            self.root, which=lambda name: "/usr/bin/g++" if name == "g++" else None,
        )
        self.assertEqual(report["audit_execution"], "PASS")
        self.assertEqual(report["inventory"]["unreal_projects"], [])
        self.assertFalse(report["unreal_tool_found_in_path"])
        self.assertEqual(report["tools"]["g++"], "/usr/bin/g++")
        self.assertTrue(all(value == "NOT RUN" for value in report["implementation_checks"].values()))
        self.assertFalse(report["git"]["worktree_dirty"])

    def test_missing_required_document_fails_the_document_check(self):
        (self.root / "AGENTS.md").unlink()
        report = audit.collect_audit(self.root, which=lambda name: None)
        self.assertEqual(report["audit_execution"], "FAIL")
        self.assertEqual(report["required_documents"]["missing"], ["AGENTS.md"])
        self.assertEqual(report["implementation_checks"]["unreal_build"], "NOT RUN")


class EnvironmentTests(unittest.TestCase):
    def test_pdf_header_is_not_content_validation(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "sample.pdf"
            path.write_bytes(b"%PDF-1.7\nnot a rendered document")
            result = audit.inspect_pdf(path)
            self.assertEqual(result["header_version"], "1.7")
            self.assertFalse(result["content_verified"])

    def test_invalid_pdf_header_is_not_given_a_version(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "broken.pdf"
            path.write_bytes(b"not a PDF")
            self.assertIsNone(audit.inspect_pdf(path)["header_version"])


if __name__ == "__main__":
    unittest.main()
