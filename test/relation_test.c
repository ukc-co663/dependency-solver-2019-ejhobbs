//
// Created by ejhobbs on 16/02/19.
//
#include "../relation.h"
#include <stdlib.h>
#include "check.h"

START_TEST(compareVersion_given_v1_equal_v2_return_0) {
    int ver1[2] = {9,9};
    int ver2[2] = {9,9};
    version v1 = {2, ver1};
    version v2 = {2, ver2};
    ck_assert_int_eq(relation_compareVersion(&v1, &v2), 0);
}
END_TEST

START_TEST(compareVersion_given_v1_smaller_return_neg) {
    int ver1[2] = {1,2};
    int ver2[2] = {1,3};
    version v1 = {2, ver1};
    version v2 = {2, ver2};
    ck_assert_int_lt(relation_compareVersion(&v1, &v2), 0);
}
END_TEST

START_TEST(compareVersion_given_v1_larger_return_pos)
{
    int ver1[2] = {1,2};
    int ver2[2] = {1,3};
    version v1 = {2, ver1};
    version v2 = {2, ver2};
    ck_assert_int_gt(relation_compareVersion(&v2, &v1), 0);
}
END_TEST

START_TEST(compareVersion_given_v1_longer_and_larger_return_pos)
    {
        int ver1[3] = {1,2,1};
        int ver2[2] = {1,2};
        version v1 = {3, ver1};
        version v2 = {2, ver2};
        ck_assert_int_gt(relation_compareVersion(&v1, &v2), 0);
    }
END_TEST

START_TEST(compareVersion_given_v2_longer_and_larger_return_neg)
    {
        int longVer[3] = {1,2,1};
        int shortVer[2] = {1,2};
        version v1 = {2, shortVer};
        version v2 = {3, longVer};
        ck_assert_int_lt(relation_compareVersion(&v1, &v2), 0);
    }
END_TEST

START_TEST(compareVersion_given_v2_longer_but_with_middle_0s_return_neg)
    {
        int longVer[3] = {1,0,1};
        int shortVer[1] = {1};
        version v1 = {1, shortVer};
        version v2 = {3, longVer};
        ck_assert_int_lt(relation_compareVersion(&v1, &v2), 0);
    }
END_TEST

START_TEST(compareVersion_given_v2_longer_but_with_0s_return_0)
    {
        int longVer[3] = {1,0,0};
        int shortVer[1] = {1};
        version v1 = {1, shortVer};
        version v2 = {3, longVer};
        ck_assert_int_eq(relation_compareVersion(&v1, &v2), 0);
    }
END_TEST

Suite* relation_suite(void) {
    Suite *s;
    TCase *tc_version;
    s = suite_create("Relation");
    tc_version = tcase_create("Compare Version");
    tcase_add_test(tc_version, compareVersion_given_v1_equal_v2_return_0);
    tcase_add_test(tc_version, compareVersion_given_v1_smaller_return_neg);
    tcase_add_test(tc_version, compareVersion_given_v1_larger_return_pos);
    tcase_add_test(tc_version, compareVersion_given_v1_longer_and_larger_return_pos);
    tcase_add_test(tc_version, compareVersion_given_v2_longer_and_larger_return_neg);
    tcase_add_test(tc_version, compareVersion_given_v2_longer_but_with_middle_0s_return_neg);
    tcase_add_test(tc_version, compareVersion_given_v2_longer_but_with_0s_return_0);

    suite_add_tcase(s, tc_version);
    return s;
}

int main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = relation_suite();
    sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS: EXIT_FAILURE;

}