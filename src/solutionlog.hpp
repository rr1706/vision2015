#ifndef SOLUTIONLOG_HPP
#define SOLUTIONLOG_HPP

#include <string>
#include <fstream>
#include <vector>
#include <map>

class SolutionLog
{
public:
    SolutionLog();
    SolutionLog(std::string file, std::vector<std::string> columns);
    bool open(std::string file, std::vector<std::string> columns);
    bool isOpen();
    SolutionLog& log(std::string columnName, double data);
    void flush();
    void close();
private:
    std::ofstream file;
    std::vector<std::string> columns;
    std::map<std::string, double> data;
    int writes;
};

#endif // SOLUTIONLOG_HPP
