#ifndef __DATATABLE_H__
#define __DATATABLE_H__
#include <Arduino.h>

template <typename T> class DataTable {
private:
    int _num_columns;
    String* _column_names;
    int _current_size;
    int _initial_size;

    int _num_rows;
    T** _data;

protected:
    void extend(void);

public:
    DataTable(int num_columns, String* column_names, int initial_size = 10);
    virtual ~DataTable();

    void append_row(int num_columns, ...);

    using FieldFormatter = String (*)(T, int);
    String get_csv_string(FieldFormatter formatter = [](T value, int col_num) -> String {
        return String(value);
    }) const;
};

template <typename T>
DataTable<T>::DataTable(int num_columns, String* column_names, int initial_size)
    :   _num_columns(num_columns),
        _column_names(column_names),
        _current_size(initial_size),
        _initial_size(initial_size),
        _num_rows(0),
        _data(NULL)
{
    _data = new T*[_current_size];
    for (int i = 0; i < _current_size; i++) {
        _data[i] = new T[_num_columns];
    }
}

template <typename T>
DataTable<T>::~DataTable() {
    for (int i = 0; i < _current_size; i++) {
        delete[] _data[i];
    }
    delete[] _data;
}

template <typename T>
void DataTable<T>::extend(void) {
    int new_size = _current_size + _initial_size;
    T** new_data = new T*[new_size];

    // copy over existing data
    for (int i = 0; i < _current_size; i++) {
        new_data[i] = _data[i];
    }

    // initialize new rows
    for (int i = _current_size; i < new_size; i++) {
        new_data[i] = new T[_num_columns];
    }

    // delete old data vector
    delete[] _data;

    // update data vector
    _data = new_data;
    _current_size = new_size;
}

template <typename T>
void DataTable<T>::append_row(int num_columns, ...) {
    if (num_columns != _num_columns) {
        return;
    }
    if (_num_rows == _current_size) {
        extend();
    }
    va_list args;
    va_start(args, num_columns);
    for (int i = 0; i < _num_columns; i++) {
        _data[_num_rows][i] = va_arg(args, T);
    }
    va_end(args);
    _num_rows++;
}

template <typename T>
String DataTable<T>::get_csv_string(FieldFormatter formatter) const {
    String csv_string = "";
    int last_column = _num_columns - 1;
    for (int i = 0; i < _num_columns; i++) {
        csv_string += _column_names[i];
        if (i < last_column) {
            csv_string += ",";
        }
    }
    csv_string += "\n";

    for (int i = 0; i < _num_rows; i++) {
        for (int j = 0; j < _num_columns; j++) {
            csv_string += formatter(_data[i][j], j);
            if (j < last_column) {
                csv_string += ",";
            }
        }
        csv_string += "\n";
    }
    return csv_string;
}

#endif // __DATATABLE_H__