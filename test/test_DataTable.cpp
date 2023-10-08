#include <Arduino.h>
#include <unity.h>
#include "test_DataTable.h"
#include "DataTable.h"


void test_DataTable(void) {
    String column_names[] = {"col1", "col2", "col3"};
    DataTable<int> dt(3, column_names);

    dt.append_row(3, 1, 2, 3);
    dt.append_row(3, 4, 5, 6);
    dt.append_row(3, 7, 8, 9);

    String csv = dt.get_csv_string();

    TEST_ASSERT_EQUAL_STRING("col1,col2,col3\n1,2,3\n4,5,6\n7,8,9\n", csv.c_str());

    String column_names2[] = {"col1", "col2", "col3", "col4"};
    DataTable<double> dt2(4, column_names2);

    dt2.append_row(4, 1.1, 2.2, 3.3, 4.4);
    dt2.append_row(4, 5.5, 6.6, 7.7, 8.8);
    dt2.append_row(4, 9.9, 10.10, 11.11, 12.12);

    String csv2 = dt2.get_csv_string();
    TEST_ASSERT_EQUAL_STRING("col1,col2,col3,col4\n1.10,2.20,3.30,4.40\n5.50,6.60,7.70,8.80\n9.90,10.10,11.11,12.12\n", csv2.c_str());    

}

void test_DataTable_extend(void) {
    String column_names[] = {"col1", "col2", "col3"};
    DataTable<int> dt(3, column_names, 3);

    dt.append_row(3, 1, 2, 3);
    dt.append_row(3, 4, 5, 6);
    dt.append_row(3, 7, 8, 9);
    dt.append_row(3, 10, 11, 12);
    dt.append_row(3, 13, 14, 15);
    dt.append_row(3, 16, 17, 18);
    dt.append_row(3, 19, 20, 21);

    String csv = dt.get_csv_string();
    TEST_ASSERT_EQUAL_STRING("col1,col2,col3\n1,2,3\n4,5,6\n7,8,9\n10,11,12\n13,14,15\n16,17,18\n19,20,21\n", csv.c_str());
}

void test_DataTable_custom_formatter(void) {
    String column_names[] = {"col1", "col2", "col3"};
    DataTable<int> dt(3, column_names);

    dt.append_row(3, 1, 2, 3);
    dt.append_row(3, 4, 5, 6);
    dt.append_row(3, 7, 8, 9);

    String csv = dt.get_csv_string([](int value, int col_num) -> String {
        if (col_num == 0) {
            return String(value * 2);
        }
        return String(value);
    });

    TEST_ASSERT_EQUAL_STRING("col1,col2,col3\n2,2,3\n8,5,6\n14,8,9\n", csv.c_str());
}