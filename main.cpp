#include <iostream>
#include <fstream>
#include <vector>

#include <unistd.h>
#include <sys/wait.h>

#include "json/json.h"

using namespace std;

typedef enum _Mode
{
    TEST,
    TC,
} Mode;

inline void print_help(string self)
{
    cout << "사용법: " << self << " [--help] [--tc] [--path PATH] binary\n"
         << '\n'
         << "인자:\n"
         << "--help: 도움말을 출력한다.\n"
         << "--tc: 테스트케이스를 출력한다.\n"
         << "--path PATH: 과제 레포지토리의 경로를 지정한다.\n"
         << '\n'
         << "binary: 실행 파일의 이름. (--path의 영향을 받음)\n";
}

inline void print_tc(Json::Value &tests)
{
    for (Json::Value test : tests)
    {
        cout << test["name"].asString() << '\n'
             << "입력:\n"
             << test["input"].asString()
             << "예상 출력:\n"
             << test["output"].asString() << '\n';
    }
}

unsigned read_config(string &path, Json::Value &config)
{
    string config_path = path + ".github/classroom/autograding.json";
    ifstream ag_config(config_path);

    if (!ag_config.is_open())
    {
        cout << "GitHub Classroom Assignment 레포지토리가 아닌 것 같습니다.\n'" << config_path << "' 파일이 존재하는지 확인해주세요.\n";
        return 1;
    }

    ag_config >> config;
    ag_config.close();

    if ((config = config["tests"]).isNull())
    {
        cout << '\'' << config_path << "' 파일 내용이 잘못되었습니다.\n";
        return 1;
    }
    return 0;
}

inline string filter_r(string s)
{
    string R("");
    for (char c : s)
        if (c != '\r')
            R += c;
    return R;
}

inline bool run(int cin, int cout, Json::Value &test)
{
    const char *input = test["input"].asCString();
    write(cin, input, strlen(input));
    close(cin);

    string output("");
    char c[1];

    while (read(cout, c, 1UL))
        output += c[0];
    close(cout);

    return filter_r(test["output"].asString()) == filter_r(output);
}

int main(int argc, char **_argv)
{
    vector<string> argv(argc);
    for (int i = 0; i < argc; i++)
        argv[i] = string(_argv[i]);

    if (argc == 1 || argv[1] == "--help")
    {
        print_help(argv[0]);
        return 0;
    }

    string path("./");
    Mode mode = TEST;
    for (int i = 0; i < argc; i++)
    {
        if (argv[i] == "--path")
        {
            path = argv[i + 1];
            if (path[path.length() - 1] != '/')
                path += '/';
        }
        else if (argv[i] == "--tc")
            mode = TC;
    }

    Json::Value tests;
    if (read_config(path, tests))
        return 0;

    switch (mode)
    {
    case TEST:
    {
        bool pass = true;
        for (Json::Value test : tests)
        {
            int ifd[2];
            int ofd[2];
            if (pipe(ifd) == -1 || pipe(ofd) == -1)
            {
                cout << "알 수 없는 오류가 발생했습니다.\n";
                return 0;
            }

            pid_t pid;
            /**
             * Child STDIN <-0 ifd 1<- Main
             * Main <-0 ofd 1<- Child STDOUT
             */
            if ((pid = fork()))
            { // Main process
                // Close 'Child'-side end.
                close(ifd[0]);
                close(ofd[1]);

                bool result = run(ifd[1], ofd[0], test);
                if (pass)
                    pass = result;

                int status;
                clock_t start = clock();
                waitpid(pid, &status, 0);
                clock_t end = clock();

                cout << test["name"].asString() << ' ' << (result ? "OK" : "FAILED") << ' ' << (((float)(end - start) / CLOCKS_PER_SEC) * 1000) << "ms" << '\n';
            }
            else
            { // Child process
                // Close 'Main'-side end.
                close(ifd[1]);
                close(ofd[0]);
                // Redirect 'Child'-side end to STDIN/STDOUT.
                dup2(ifd[0], STDIN_FILENO);
                dup2(ofd[1], STDOUT_FILENO);
                // Close 'Child'-side end.
                close(ifd[0]);
                close(ofd[1]);

                execl((path + argv[argc - 1]).c_str(), "", nullptr);
                return 1;
            }
        }
        cout << (pass ? "맞았습니다." : "틀렸습니다.") << '\n';
    }
    break;
    case TC:
        print_tc(tests);
        break;
    }
    return 0;
}