//
// Created by ejhobbs on 16/02/19.
//
#include "../repository.h"
#include <stdlib.h>
#include "check.h"
START_TEST(getPackageIndex_given_name_and_specified_version_not_exists_return_neg) {
    //pkg1
    int v1[3] = {0,1,2};
    version V1 = {3, v1};
    package pkg1 = {"d", 0, V1, 0, NULL, 0, NULL};

    //pkg2
    int v2[3] = {1,0,2};
    version V2 = {3, v2};
    package pkg2 = {"d", 0, V2, 0, NULL, 0, NULL};

    //pkg3
    int v3[3] = {0,3,2};
    version V3 = {3, v3};
    package pkg3 = {"pkg", 0, V3, 0, NULL, 0, NULL};

    //repo
    package* pkgs[3] = {&pkg1,&pkg2, &pkg3};
    repo_repository r = {3, pkgs, NULL};
    int v4[4] = {1,3,5,3};
    version v = {4, v4};
    relation rel = {"pkg", v, _eq};
    ck_assert_int_eq(repo_getPackageIndex(&r, &rel), -1);
}
END_TEST
START_TEST(getPackageIndex_given_name_and_specified_version_return_that) {
    //pkg1
    int v1[3] = {0,1,2};
    version V1 = {3, v1};
    package pkg1 = {"d", 0, V1, 0, NULL, 0, NULL};

    //pkg2
    int v2[3] = {1,0,2};
    version V2 = {3, v2};
    package pkg2 = {"d", 0, V2, 0, NULL, 0, NULL};

    //pkg3
    int v3[3] = {0,3,2};
    version V3 = {3, v3};
    package pkg3 = {"pkg", 0, V3, 0, NULL, 0, NULL};

    //repo
    package* pkgs[3] = {&pkg1,&pkg2, &pkg3};
    repo_repository r = {3, pkgs, NULL};
    relation rel = {"d", V2, _eq};
    ck_assert_int_eq(repo_getPackageIndex(&r,&rel), 1);
}
END_TEST

START_TEST(getPackageIndex_given_name_and_null_version_return_first) {
    //pkg1
    int v1[3] = {0,1,2};
    version V1 = {3, v1};
    package pkg1 = {"d", 0, V1, 0, NULL, 0, NULL};

    //pkg2
    int v2[3] = {1,0,2};
    version V2 = {3, v2};
    package pkg2 = {"d", 0, V2, 0, NULL, 0, NULL};

    //pkg3
    int v3[3] = {0,3,2};
    version V3 = {3, v3};
    package pkg3 = {"pkg", 0, V3, 0, NULL, 0, NULL};

    //repo
    package* pkgs[3] = {&pkg1,&pkg2, &pkg3};
    repo_repository r = {3, pkgs, NULL};
    version v = {0, NULL};
    relation rel = {"d", v, _eq};
    ck_assert_int_eq(repo_getPackageIndex(&r, &rel), 0);
}
END_TEST

START_TEST(getPackageIndex_given_no_name_return_neg) {
    //pkg1
    int v1[3] = {0,1,2};
    version V1 = {3, v1};
    package pkg1 = {"d", 0, V1, 0, NULL, 0, NULL};

    //pkg2
    int v2[3] = {1,0,2};
    version V2 = {3, v2};
    package pkg2 = {"d", 0, V2, 0, NULL, 0, NULL};

    //pkg3
    int v3[3] = {0,3,2};
    version V3 = {3, v3};
    package pkg3 = {"pkg", 0, V3, 0, NULL, 0, NULL};

    //repo
    package* pkgs[3] = {&pkg1,&pkg2, &pkg3};
    repo_repository r = {3, pkgs, NULL};
    relation rel = {"not_exists", V2, _eq};
    ck_assert_int_eq(repo_getPackageIndex(&r, &rel), -1);
}
END_TEST

START_TEST(getPackageIndex_given_multiple_name_and_version_matches_lt_return_first_pos) {
    //pkg1
    int v1[3] = {0,1,2};
    version V1 = {3, v1};
    package pkg1 = {"d", 0, V1, 0, NULL, 0, NULL};

    //pkg2
    int v2[3] = {1,0,2};
    version V2 = {3, v2};
    package pkg2 = {"d", 0, V2, 0, NULL, 0, NULL};

    //pkg3
    int v3[3] = {0,3,2};
    version V3 = {3, v3};
    package pkg3 = {"pkg", 0, V3, 0, NULL, 0, NULL};

    //repo
    package* pkgs[3] = {&pkg1,&pkg2, &pkg3};
    repo_repository r = {3, pkgs, NULL};
    relation rel = {"d", V2, _lt};
    ck_assert_int_eq(repo_getPackageIndex(&r, &rel), 0);
}
END_TEST

START_TEST(getPackageIndex_given_multiple_name_and_version_matches_gt_return_first_pos) {
    //pkg1
    int v1[3] = {0,1,2};
    version V1 = {3, v1};
    package pkg1 = {"d", 0, V1, 0, NULL, 0, NULL};

    //pkg2
    int v2[3] = {1,0,2};
    version V2 = {3, v2};
    package pkg2 = {"d", 0, V2, 0, NULL, 0, NULL};

    //pkg3
    int v3[3] = {0,3,2};
    version V3 = {3, v3};
    package pkg3 = {"pkg", 0, V3, 0, NULL, 0, NULL};

    //repo
    package* pkgs[3] = {&pkg1,&pkg2, &pkg3};
    repo_repository r = {3, pkgs, NULL};
    relation rel = {"d", V1, _gt};
    ck_assert_int_eq(repo_getPackageIndex(&r, &rel), 1);
}
END_TEST

Suite* repository_suite(void) {
    Suite *s;
    TCase *tc_package;
    s = suite_create("Repository");
    tc_package = tcase_create("Find package");
    tcase_add_test(tc_package, getPackageIndex_given_no_name_return_neg);
    tcase_add_test(tc_package, getPackageIndex_given_name_and_null_version_return_first);
    tcase_add_test(tc_package, getPackageIndex_given_name_and_specified_version_return_that);
    tcase_add_test(tc_package, getPackageIndex_given_name_and_specified_version_not_exists_return_neg);
    tcase_add_test(tc_package, getPackageIndex_given_multiple_name_and_version_matches_lt_return_first_pos);
    tcase_add_test(tc_package, getPackageIndex_given_multiple_name_and_version_matches_gt_return_first_pos);
    suite_add_tcase(s, tc_package);
    return s;
}

int main(void) {
    /* test setup */
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = repository_suite();
    sr = srunner_create(s);
    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS: EXIT_FAILURE;

}
