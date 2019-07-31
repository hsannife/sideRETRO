#include "config.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <check.h>
#include "check_sider.h"

#include "../src/log.h"
#include "../src/wrapper.h"

START_TEST (test_xmalloc)
{
	void *p = xmalloc (sizeof (int));
	* (int *) p = 66;
	ck_assert_int_eq (* (int *) p, 66);
	xfree (p);

	p = xmalloc (0);
	* (char *) p = 'p';
	ck_assert_int_eq (* (char *) p, 'p');
	xfree (p);
}
END_TEST

START_TEST (test_xcalloc)
{
	void *p = xcalloc (1, sizeof (int));
	ck_assert_int_eq (* (int *) p, 0);
	xfree (p);

	p = xcalloc (0, sizeof (int));
	* (char *) p = 'p';
	ck_assert_int_eq (* (char *) p, 'p');
	xfree (p);

	p = xcalloc (1, 0);
	* (char *) p = 'p';
	ck_assert_int_eq (* (char *) p, 'p');
	xfree (p);
}
END_TEST

START_TEST (test_xrealloc)
{
	void *p = NULL;

	p = xrealloc (p, sizeof (int));
	* (int *) p = 66;
	ck_assert_int_eq (* (int *) p, 66);

	p = xrealloc (p, sizeof (int) * 2);
	* ((int *) p + 1) = 33;
	ck_assert_int_eq (* ((int *) p + 1), 33);

	xfree (p);
}
END_TEST

START_TEST (test_xstrdup)
{
	char *s = xstrdup ("PONGA");
	ck_assert_str_eq (s, "PONGA");
	xfree (s);
}
END_TEST

START_TEST (test_xfree)
{
	void *p = xmalloc (1);
	xfree (p);
	xfree (NULL);
}
END_TEST

START_TEST (test_xfopen)
{
	char path[] = "/tmp/pongaXXXXXX";
	int fd = xmkstemp (path);
	close (fd);

	FILE *fp = xfopen (path, "w");
	ck_assert (fp != NULL);
	xfclose (fp);

	fp = xfopen (path, "r");
	ck_assert (fp != NULL);
	xfclose (fp);

	xunlink (path);
}
END_TEST

START_TEST (test_xfopen_rw_abort)
{
	FILE *fp = xfopen (NULL, "r+");
}
END_TEST

START_TEST (test_xfopen_w_abort)
{
	FILE *fp = xfopen (NULL, "w");
}
END_TEST

START_TEST (test_xfopen_r_abort)
{
	FILE *fp = xfopen (NULL, "r");
}
END_TEST

START_TEST (test_xfdopen)
{
	char path[] = "/tmp/pongaXXXXXX";
	int fd = xmkstemp (path);

	FILE *fp = xfdopen (fd, "w");
	ck_assert (fp != NULL);
	xfclose (fp);

	xunlink (path);
}
END_TEST

START_TEST (test_xfdopen_rw_abort)
{
	FILE *fp = xfdopen (-10, "r+");
}
END_TEST

START_TEST (test_xfdopen_w_abort)
{
	FILE *fp = xfdopen (-10, "w");
}
END_TEST

START_TEST (test_xfdopen_r_abort)
{
	FILE *fp = xfdopen (-10, "r");
}
END_TEST

START_TEST (test_xfclose)
{
	xfclose (NULL);
}
END_TEST

START_TEST (test_xfflush)
{
	char buf[10];
	const char *str = "PONGA";
	int str_size = strlen (str);
	int size = 0;

	char path[] = "/tmp/pongaXXXXXX";
	int fd = xmkstemp (path);

	FILE *fp = xfdopen (fd, "w+");
	size = fwrite (str, sizeof (char), str_size, fp);

	xfflush (fp);
	fseek (fp, 0L, SEEK_SET);

	size = fread (buf, sizeof (char), str_size, fp);
	buf[str_size] = '\0';

	ck_assert_str_eq (buf, str);

	xfclose (fp);
	xunlink (path);
}
END_TEST

START_TEST (test_xpopen)
{
	char path[] = "/tmp/pongaXXXXXX";
	char buf[BUFSIZ];
	int rc = 0;

	FILE *pp = xpopen ("dir", "r");

	while (fgets (buf, BUFSIZ, pp) != NULL)
		;

	rc = xpclose (pp);
	ck_assert_int_eq (rc, 0);

	int fd = xmkstemp (path);
	close (fd);

	xsnprintf (buf, BUFSIZ - 1, "cat > %s", path);
	buf[BUFSIZ - 1] = '\0';

	pp = xpopen (buf, "w");
	xfputs ("PONGA", pp);

	rc = xpclose (pp);
	ck_assert_int_eq (rc, 0);
}
END_TEST

START_TEST (test_xpopen_r_abort)
{
	FILE *pp = xpopen ("poooooooga 2> /dev/null", "r");
	ck_assert_int_ne (xpclose (pp), 0);
}
END_TEST

START_TEST (test_xpopen_w_abort)
{
	FILE *pp = xpopen ("poooooooga 2> /dev/null", "w");
	ck_assert_int_ne (xpclose (pp), 0);
}
END_TEST

START_TEST (test_xmkdir)
{
	DIR *dp = NULL;
	const char dir[] = "/tmp/ponga";

	xmkdir (dir, S_IRWXU);

	dp = opendir (dir);
	ck_assert (dp != NULL);
	closedir (dp);

	// Do nothing
	xmkdir (dir, S_IRWXU);
	rmdir (dir);
}
END_TEST

START_TEST (test_xmkdir_abort)
{
	xmkdir ("/tmp/tmp/tmp/tmp", S_IRWXU);
}
END_TEST

START_TEST (test_xasprintf)
{
	char *s = NULL;
	char t[] = "PONGA";

	xasprintf (&s, "%s", t);
	ck_assert_str_eq (s, t);

	xfree (s);
}
END_TEST

START_TEST (test_xfprintf)
{
	char buf[10];
	const char *str = "PONGA";
	int str_size = strlen (str);
	int size = 0;

	char path[] = "/tmp/pongaXXXXXX";
	int fd = xmkstemp (path);

	FILE *fp = xfdopen (fd, "w");
	size = xfprintf (fp, "%s", str);
	xfclose (fp);

	fp = xfopen (path, "r");
	size = fread (buf, sizeof (char), str_size, fp);
	buf[str_size] = '\0';

	ck_assert_str_eq (buf, str);

	xfclose (fp);
	xunlink (path);
}
END_TEST

Suite *
make_wrapper_suite (void)
{
	log_set_quiet (1);

	Suite *s;
	TCase *tc_core;
	TCase *tc_abort;

	s = suite_create ("Wrapper");

	/* Core test case */
	tc_core = tcase_create ("Core");

	/* Abort test case */
	tc_abort = tcase_create ("Abort");

	tcase_add_test (tc_core, test_xmalloc);
	tcase_add_test (tc_core, test_xcalloc);
	tcase_add_test (tc_core, test_xrealloc);
	tcase_add_test (tc_core, test_xstrdup);
	tcase_add_test (tc_core, test_xfree);
	tcase_add_test (tc_core, test_xfopen);
	tcase_add_test (tc_core, test_xfdopen);
	tcase_add_test (tc_core, test_xfclose);
	tcase_add_test (tc_core, test_xfflush);
	tcase_add_test (tc_core, test_xpopen);
	tcase_add_test (tc_core, test_xmkdir);
	tcase_add_test (tc_core, test_xasprintf);
	tcase_add_test (tc_core, test_xfprintf);

	tcase_add_test_raise_signal (tc_abort, test_xfopen_rw_abort, 6);
	tcase_add_test_raise_signal (tc_abort, test_xfopen_w_abort, 6);
	tcase_add_test_raise_signal (tc_abort, test_xfopen_r_abort, 6);
	tcase_add_test_raise_signal (tc_abort, test_xfdopen_rw_abort, 6);
	tcase_add_test_raise_signal (tc_abort, test_xfdopen_w_abort, 6);
	tcase_add_test_raise_signal (tc_abort, test_xfdopen_r_abort, 6);
	tcase_add_test (tc_abort, test_xpopen_r_abort);
	tcase_add_test (tc_abort, test_xpopen_w_abort);
	tcase_add_test_raise_signal (tc_abort, test_xmkdir_abort, 6);

	suite_add_tcase (s, tc_core);
	suite_add_tcase (s, tc_abort);

	return s;
}
