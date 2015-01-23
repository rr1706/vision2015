#include "solutionlog.hpp"

SolutionLog::SolutionLog() : writes(0)
{
}

SolutionLog::SolutionLog(std::string file, std::vector<std::string> columns) : writes(0)
{
    this->open(file, columns);
}

bool SolutionLog::open(std::string file, std::vector<std::string> columns)
{
    this->file.open(file.c_str());
    if (this->file.is_open()) {
        for (std::string column : columns) {
            this->file << column << ",";
        }
        this->file << std::endl;
    } else {
        throw std::runtime_error("Failed to open solution log");
    }
    this->columns = columns;
    this->data.clear();
    for (std::string column : this->columns) {
        this->data[column] = 0;
    }
    return this->file.is_open();
}

bool SolutionLog::isOpen() {
    return this->file.is_open();
}

void SolutionLog::close() {
    this->file.close();
}

SolutionLog& SolutionLog::log(std::string columnName, double data)
{
    this->data[columnName] = data;
    return *this;
}

void SolutionLog::flush()
{
    for (std::string column : this->columns) {
        this->file << this->data[column] << ",";
    }
    this->data.clear();
    for (std::string column : this->columns) {
        this->data[column] = 0;
    }
    if (this->writes > 20) { // write to the file every 20 solutions
        this->writes = 0;
        this->file << std::endl;
    } else {
        this->file << '\n';
        this->writes++;
    }
}
