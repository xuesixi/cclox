16章，扫描器

Scanner负责将源代码转化为一个个的token。在本解释器的实现中，scanner并不会在一开始就把整个源文件全部转化为一群token然后交给Compiler（在jlox中是这么做的）。相反，这个实现中，Scanner和Compiler是同时存在的。每当compiler需要一个token的时候，它就会让Scanner扫描下一个token。

## Token
``` c++
class Token {
    std::string lexeme; // token所对应的字符串。为了简化生命周期的考虑，token会有自己独立的拷贝
    TokenType type; // token类型
    int line; // 行号
}
```

## Scanner
```c++
class Scanner {
    const std::string text; // 整个源代码文本
    size_t start_index; // 当前token的开头
    size_t next_index; // 下一个要扫描的字符
    int curr_line; // 当前行号
};
```
它最重要的函数是`scan_token()`，返回下一个token。除此之外是一系列用来实现该功能的辅助函数.
```c++
Token Scanner::make_token(TokenType type); // 以start_index和next_index范围中的字符创建一个指定类型的token
char Scanner::advance(); // 返回next_index所指的字符，并自增next_index
char Scanner::peek_next(size_t n = 0);// 返回text[next_index + n]处的字符。
bool Scanner::match(char c); // 判断next_index处的字符是否为给定的字符。如果EOF或者不匹配，返回false，否则自增next_index，并返回true
bool Scanner::is_at_end(); // 判断是否为EOF
void Scanner::skip_whitespaces(); // 跳过所有空白字符，包括空格、tab、换行符、注释。
```

`scan_token()`的逻辑大约如下：
```c++
Token Scanner::scan_token() {
    skip_whitespaces();
    if (is_at_end()) {
        return make_token(TokenType::EOF);
    }
    start_index = next_index;
    char c = advance();
    switch (c) {
        // 根据c的值，返回不同的token
    }
}
```
在生成token的时候，根据我们看到的第一个字符（上面代码中的`c`），大体上分为三种：
* 单字符token，例如`+`，看到第一个字符，就确定它是`TokenType::Add`
* 单或双字符token。例如`=`，可能是一个单独的`=`, 也看是一个`==`，需要对下一个字符进行匹配判断
* 多字符token，例如字符串、数字、标识符。它们有特殊的函数进行处理
    * 原书中在处理标识符的时候，用到了trie结构来区分关键字和普通标识符。但c++中自带哈希表结构，因此省略了这部分的实现。

行号会在`skip_whitespaces()`和`scan_string()`中进行处理。