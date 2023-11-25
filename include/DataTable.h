#ifndef __DATATABLE_H__
#define __DATATABLE_H__
#include <Arduino.h>

// DataTable is a class that stores data in a table and then can output the contents as a CSV. Class is designed to
// collect data as efficiently as possible, then later output the data as a CSV when timing is not so critical.
//
// The table has a fixed number of columns of a consistent type, but the number of rows can grow dynamically. The
// table is initialized with a set of column names. Rows are added  to the table using the append_row method. The
// append_row method takes the number of columns as th first argument, followed by the values for each column.
//
template <typename T> class DataTable {
private:
    int _num_columns;
    String* _column_names;
    int _current_size;
    int _initial_size;

    int _num_rows;
    T** _data;

protected:
    // extends the storage for the table by _initial_size rows. returns true if the storage was extended, false
    // otherwise.
    bool extend(void);

public:
    DataTable(int num_columns, String* column_names, int initial_size = 10);
    DataTable(const DataTable<T>& other);
    virtual ~DataTable();

    // adds a row to the table. The number of columns must match the number of columns in the table.
    // returns true if the row was added, false otherwise.
    bool append_row(int num_columns, ...);

    using FieldFormatter = String (*)(T, int);
    void write_to_stream(Stream& stream, FieldFormatter formatter = [](T value, int col_num) -> String {
        return String(value);
    }) const;
};

template <typename T>
DataTable<T>::DataTable(int num_columns, String* column_names, int initial_size)
    :   _num_columns(num_columns),
        _column_names(nullptr),
        _current_size(initial_size),
        _initial_size(initial_size),
        _num_rows(0),
        _data(nullptr)
{
    _column_names = new String[_num_columns];
    for (int i = 0; i < _num_columns; i++) {
        _column_names[i] = column_names[i];
    }

    _data = new T*[_current_size];
    for (int i = 0; i < _current_size; i++) {
        _data[i] = new T[_num_columns];
    }
}

template <typename T>
DataTable<T>::DataTable(const DataTable<T>& other)
    :   _num_columns(other._num_columns),
        _column_names(nullptr),
        _current_size(other._current_size),
        _initial_size(other._initial_size),
        _num_rows(other._num_rows),
        _data(nullptr)
{
    _column_names = new String[_num_columns];
    for (int i = 0; i < _num_columns; i++) {
        _column_names[i] = other._column_names[i];
    }
    _data = new T*[_current_size];
    for (int i = 0; i < _current_size; i++) {
        _data[i] = new T[_num_columns];
        if (i < _num_rows ) {
            for (int j = 0; j < _num_columns; j++) {
                _data[i][j] = other._data[i][j];
            }
        }
    }
}

template <typename T>
DataTable<T>::~DataTable() {
    for (int i = 0; i < _current_size; i++) {
        delete[] _data[i];
    }
    delete[] _data;
    delete[] _column_names;
}

template <typename T>
bool DataTable<T>::extend(void) {
    int new_size = _current_size + _initial_size;
    T** new_data = new T*[new_size];
    if (new_data == NULL) {
        return false;
    }
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

    return true;
}

template <typename T>
bool DataTable<T>::append_row(int num_columns, ...) {
    if (num_columns != _num_columns) {
        return false;
    }
    if (_num_rows == _current_size) {
        if (!extend()) {
            return false;
        }
    }
    va_list args;
    va_start(args, num_columns);
    for (int i = 0; i < _num_columns; i++) {
        _data[_num_rows][i] = va_arg(args, T);
    }
    va_end(args);
    _num_rows++;
    return true;
}

template <typename T>
void DataTable<T>::write_to_stream(Stream& stream, FieldFormatter formatter) const {
    int last_column = _num_columns - 1;
    for (int i = 0; i < _num_columns; i++) {
        stream.print(_column_names[i]);
        if (i < last_column) {
            stream.print(",");
        }
    }
    stream.println("");
    stream.flush();

    for (int i = 0; i < _num_rows; i++) {
        for (int j = 0; j < _num_columns; j++) {
            stream.print(formatter(_data[i][j], j));
            if (j < last_column) {
                stream.print(",");
            }
        }
        stream.println("");
        stream.flush();
    }
}

#endif // __DATATABLE_H__