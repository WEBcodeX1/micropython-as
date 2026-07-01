static const ServerFile f1 = { "/index.html", "text/html", file1, 122 };
static const ServerFile f2 = { "/404.html", "text/html", file2, 55 };
static const ServerFile f3 = { "/favicon.ico", "img/png", file3, 1937 };
static const ServerFile f4 = { "/falcon-robotics.jpg", "img/jpg", file4, 118236 };
static const std::array<ServerFile, 4> ServerFiles = { f1, f2, f3, f4, };