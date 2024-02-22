#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <getopt.h>
#include <regex>
#include <string>
#include <vector>

constexpr int MS_PER_HOUR = 3600000;
constexpr int MS_PER_MINUTE = 60000;
constexpr int MS_PER_SECOND = 1000;

struct TimeDelta
{
    int hours;
    int minutes;
    int seconds;
    int milliseconds;

    int toMilliseconds() const
    {
        return ((hours * 60 + minutes) * 60 + seconds) * 1000 + milliseconds;
    }

    static TimeDelta fromMilliseconds(int milliseconds)
    {
        TimeDelta td;
        td.hours = milliseconds / MS_PER_HOUR;
        milliseconds %= MS_PER_HOUR;
        td.minutes = milliseconds / MS_PER_MINUTE;
        milliseconds %= MS_PER_MINUTE;
        td.seconds = milliseconds / MS_PER_SECOND;
        td.milliseconds = milliseconds % MS_PER_SECOND;
        return td;
    }

    TimeDelta operator-(const TimeDelta &other) const
    {
        int delta_ms = toMilliseconds() - other.toMilliseconds();
        return fromMilliseconds(delta_ms);
    }

    TimeDelta operator/(int divisor) const
    {
        int total_ms = toMilliseconds() / divisor;
        return fromMilliseconds(total_ms);
    }

    TimeDelta operator+(const TimeDelta &other) const
    {
        int sum_ms = toMilliseconds() + other.toMilliseconds();
        return fromMilliseconds(sum_ms);
    }
};

TimeDelta parseSRTTime(const std::string &time_str)
{
    std::regex re("[:,]");
    std::sregex_token_iterator it(time_str.begin(), time_str.end(), re, -1);
    std::vector<std::string> time_parts(it, {});

    TimeDelta td;
    td.hours = std::stoi(time_parts[0]);
    td.minutes = std::stoi(time_parts[1]);
    td.seconds = std::stoi(time_parts[2]);
    td.milliseconds = std::stoi(time_parts[3]);

    return td;
}

std::string timeDeltaToString(const TimeDelta &td)
{
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << td.hours << ":"
       << std::setw(2) << std::setfill('0') << td.minutes << ":"
       << std::setw(2) << std::setfill('0') << td.seconds << ","
       << std::setw(3) << std::setfill('0') << td.milliseconds;
    return ss.str();
}

void processSRTFile(const std::string &input_file, const std::string &output_file)
{
    std::ifstream infile(input_file);
    std::ofstream outfile(output_file);
    if (!infile.is_open() || !outfile.is_open())
    {
        std::cout << "Could not open files" << std::endl;
        return;
    }

    std::string line;
    int i = 1;
    while (std::getline(infile, line))
    {
        if (std::regex_match(line, std::regex("\\d+")))
        {
            std::getline(infile, line);
            std::string start_time_str, end_time_str;
            std::istringstream iss(line);
            std::getline(iss, start_time_str, ' ');

            // skip " --> "

            std::getline(iss, line, ' ');
            std::getline(iss, end_time_str);

            TimeDelta start_time = parseSRTTime(start_time_str);
            TimeDelta end_time = parseSRTTime(end_time_str);
            TimeDelta duration = end_time - start_time;

            std::getline(infile, line);
            std::istringstream wordStream(line);
            std::vector<std::string> words;
            std::string word;
            while (wordStream >> word)
            {
                words.push_back(word);
            }
            int num_words = words.size();
            TimeDelta time_per_word = duration / num_words;

            TimeDelta current_time = start_time;
            for (const auto &w : words)
            {
                TimeDelta next_time = current_time + time_per_word;
                outfile << i << std::endl;
                outfile << timeDeltaToString(current_time) << " --> " << timeDeltaToString(next_time) << std::endl;
                outfile << w << std::endl
                        << std::endl;

                ++i;
                current_time = next_time;
            }
        }
    }

    infile.close();
    outfile.close();
}

int main(int argc, char *argv[])
{
    std::string input_file;
    std::string output_file;

    static struct option long_options[] = {
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {0, 0, 0, 0}};

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "i:o:", long_options, &option_index)) != -1)
    {
        switch (c)
        {
        case 'i':
            input_file = optarg;
            break;
        case 'o':
            output_file = optarg;
            break;
        default:
            std::cout << "Usage: " << argv[0] << " --input <input_file> --output <output_file>" << std::endl;
            return 1;
        }
    }

    if (input_file.empty() || output_file.empty())
    {
        std::cout << "Usage: " << argv[0] << " --input <input_file> --output <output_file>" << std::endl;
        return 1;
    }

    processSRTFile(input_file, output_file);
    return 0;
}
