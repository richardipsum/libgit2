#include "clar_libgit2.h"
#include "commit.h"
#include "git2/commit.h"
#include "../merge/merge_helpers.h"

static const char *merge_reflog_message = "commit (merge): Merge commit";

static git_repository *_repo;

void test_commit_merge__initialize(void)
{
	_repo = cl_git_sandbox_init("merge-recursive");
}

void test_commit_merge__cleanup(void)
{
	cl_git_sandbox_cleanup();
}

void test_commit_merge__merge_commit_shows_in_reflog(void)
{
	git_index *index;
	git_merge_options merge_opts = GIT_MERGE_OPTIONS_INIT;
	git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
	git_oid b1_oid;
	git_oid b2_oid;
	git_oid tree_oid;
	git_oid merge_commit_oid;
	git_commit *b1_commit;
	git_commit *b2_commit;
	git_signature *s;
	git_commit *parent_commits[2];
	git_tree *tree;
	git_reflog *log;
	const git_reflog_entry *entry;

	cl_git_pass(git_signature_now(&s, "alice", "alice@example.com"));

	cl_git_pass(git_reference_name_to_id(&b1_oid, _repo, "refs/heads/branchB-1"));
	cl_git_pass(git_reference_name_to_id(&b2_oid, _repo, "refs/heads/branchB-2"));

	cl_git_pass(git_commit_lookup(&b1_commit, _repo, &b1_oid));
	cl_git_pass(git_commit_lookup(&b2_commit, _repo, &b2_oid));

	parent_commits[0] = b1_commit;
	parent_commits[1] = b2_commit;

	cl_git_pass(merge_branches(_repo,
		"refs/heads/branchB-1", "refs/heads/branchB-2",
		&merge_opts, &checkout_opts));

	cl_git_pass(git_repository_index(&index, _repo));
	cl_git_pass(git_index_read(index, 1));
	cl_assert_(!git_index_has_conflicts(index), "Index has conflicts!");
	cl_git_pass(git_index_write_tree(&tree_oid, index));
	cl_git_pass(git_tree_lookup(&tree, _repo, &tree_oid));

	cl_git_pass(git_commit_create(&merge_commit_oid,
		_repo, "refs/heads/branchB-1", s, s, NULL,
		"Merge commit", tree,
		2, (const struct git_commit **) parent_commits));

	cl_git_pass(git_reflog_read(&log, _repo, "refs/heads/branchB-1"));
	entry = git_reflog_entry_byindex(log, 0);
	cl_assert_equal_s(merge_reflog_message, git_reflog_entry_message(entry));
}
