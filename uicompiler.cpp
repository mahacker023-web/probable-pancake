#include <windows.h>
#include <string>
#include <sstream>
#include <map>

// UI Elements کے لیے گلوبل ہینڈلز
HWND hCodeBox, hOutputBox, hStatusBox;
HFONT hFont;

// ویری ایبلز کو میموری میں سیو کرنے کے لیے Map (جیسے: x = 5)
std::map<std::string, std::string> variables;

// اسٹرنگ کے شروع اور آخر سے فالتو اسپیسز ختم کرنے کا فنکشن
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// MalikLang کا اصل کمپائلر لاجک (Core Engine)
std::string processLine(std::string line) {
    line = trim(line);
    if (line.empty() || line.rfind("//", 0) == 0) return ""; // کمنٹس کو نظر انداز کرنا

    // 1. PRINT کمانڈ کا لاجک
    if (line.rfind("print ", 0) == 0) {
        std::string value = trim(line.substr(6));
        
        // اگر ڈائریکٹ ٹیکسٹ ہے: print "Hello"
        if (value.front() == '"' && value.back() == '"') {
            return value.substr(1, value.length() - 2) + "\r\n";
        }
        // اگر کوئی ویری ایبل پرنٹ کرنا ہے: print myVar
        else if (variables.find(value) != variables.end()) {
            return variables[value] + "\r\n";
        }
        return "Runtime Error: '" + value + "' matrix ya text sahi nahi hai!\r\n";
    }

    // 2. VARIABLE ASSIGNMENT کا لاجک (جیسے: set count = 100)
    if (line.rfind("set ", 0) == 0) {
        size_t eqPos = line.find('=');
        if (eqPos != std::string::npos) {
            std::string varName = trim(line.substr(4, eqPos - 4));
            std::string varValue = trim(line.substr(eqPos + 1));
            
            // اگر ویلیو ٹیکسٹ ہے
            if (varValue.front() == '"' && varValue.back() == '"') {
                variables[varName] = varValue.substr(1, varValue.length() - 2);
            } else {
                variables[varName] = varValue; // نمبر یا سادہ ویلیو
            }
            return ""; // ویری ایبل سیٹ ہونے پر کوئی آؤٹ پٹ نہیں دکھانا
        }
        return "Syntax Error: '=' missing in variable assignment!\r\n";
    }

    // 3. MATH OPERATIONS (جیسے: calc 25 + 75)
    if (line.rfind("calc ", 0) == 0) {
        std::string expr = trim(line.substr(5));
        std::stringstream ss(expr);
        long long num1 = 0, num2 = 0;
        char op;
        if (ss >> num1 >> op >> num2) {
            long long res = 0;
            if (op == '+') res = num1 + num2;
            else if (op == '-') res = num1 - num2;
            else if (op == '*') res = num1 * num2;
            else if (op == '/') {
                if (num2 == 0) return "Math Error: Division by zero!\r\n";
                res = num1 / num2;
            }
            return "Calc Result: " + std::to_string(res) + "\r\n";
        }
        return "Syntax Error: Invalid math format! Use 'calc 10 + 20'\r\n";
    }

    return "Syntax Error: '" + line + "' command samajh nahi aayi!\r\n";
}

// پورے کوڈ بلاک کو لائن بائی لائن چلانے کا فنکشن
std::string executeMalikLang(std::string fullCode) {
    variables.clear(); // ہر بار رن کرنے پر پرانی میموری صاف کرنا
    std::string finalOutput = "";
    std::stringstream ss(fullCode);
    std::string line;

    while (std::getline(ss, line, '\n')) {
        finalOutput += processLine(line);
    }
    return finalOutput.empty() ? "Code executed successfully with no output." : finalOutput;
}

// UI ونڈو کے ایکشنز کا ہینڈلر
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_COMMAND:
            if (wp == 1) { // Run Button Logic
                int len = GetWindowTextLengthA(hCodeBox);
                std::string code(len, '\0');
                GetWindowTextA(hCodeBox, &code[0], len + 1);
                
                SetWindowTextA(hStatusBox, "Compiling & Running...");
                std::string output = executeMalikLang(code);
                SetWindowTextA(hOutputBox, output.c_str());
                SetWindowTextA(hStatusBox, "Status: Ready");
            }
            if (wp == 2) { // Clear Button Logic
                SetWindowTextA(hCodeBox, "");
                SetWindowTextA(hOutputBox, "");
                SetWindowTextA(hStatusBox, "Status: Cleared");
            }
            break;

        case WM_CREATE:
            // فاؤنٹ سیٹ کرنا تاکہ ٹیکسٹ خوبصورت دیکھے
            hFont = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");

            // UI Labels & Boxes
            CreateWindowA("STATIC", " Write your MalikLang Code:", WS_VISIBLE | WS_CHILD | SS_LEFT, 20, 10, 440, 20, hwnd, NULL, NULL, NULL);
            
            // کوڈ ان پٹ باکس (Multiline Text Area)
            hCodeBox = CreateWindowA("EDIT", "set name = \"Mubashir Ali\"\nprint name\n\ncalc 50 * 4\n// custom comment line", 
                                     WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL, 
                                     20, 35, 440, 120, hwnd, NULL, NULL, NULL);
            SendMessageA(hCodeBox, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Control Buttons
            CreateWindowA("BUTTON", "▶ Run Code", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 20, 165, 120, 35, hwnd, (HMENU)1, NULL, NULL);
            CreateWindowA("BUTTON", "🗑 Clear", WS_VISIBLE | WS_CHILD, 150, 165, 100, 35, hwnd, (HMENU)2, NULL, NULL);

            // اسٹیٹس بار
            hStatusBox = CreateWindowA("STATIC", "Status: Ready", WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTERIMAGE, 260, 165, 200, 35, hwnd, NULL, NULL, NULL);

            // آؤٹ پٹ باکس
            CreateWindowA("STATIC", " Console Output:", WS_VISIBLE | WS_CHILD, 20, 215, 440, 20, hwnd, NULL, NULL, NULL);
            hOutputBox = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL, 
                                       20, 240, 440, 150, hwnd, NULL, NULL, NULL);
            SendMessageA(hOutputBox, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;

        case WM_DESTROY:
            DeleteObject(hFont);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}

// ونڈوز ایپلیکیشن مین انٹری پوائنٹ
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow) {
    WNDCLASSW wc = {0};
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1); // خوبصورت گرے بیک گراؤنڈ
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInst;
    wc.lpszClassName = L"MalikLangStudio";
    wc.lpfnWndProc = WindowProcedure;

    if (!RegisterClassW(&wc)) return -1;
    
    // اسکرین کے سینٹر میں ونڈو کھولنا
    CreateWindowW(L"MalikLangStudio", L"MalikLang Professional UI Studio v2.0", 
                  WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, 
                  250, 150, 495, 440, NULL, NULL, hInst, NULL);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
