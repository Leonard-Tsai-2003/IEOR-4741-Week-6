//g++ -std=c++17 -O2 -Iinclude hw6_policy.cpp -o /tmp/w6_policy && /tmp/w6_policy

#include <iostream>
#include <string>

// --- Policy 1: Plain Text Formatting ---
struct PlainTextPolicy {
    void write_output(const std::string& title, const std::string& body) const {
        std::cout << title << "\n" << body << "\n";
    }
};

// --- Policy 2: Markdown Formatting ---
struct MarkdownPolicy {
    void write_output(const std::string& title, const std::string& body) const {
        std::cout << "# " << title << "\n\n" << body << "\n";
    }
};

// --- Host Class ---
// Uses C++20 [[no_unique_address]] (or private inheritance) for Empty Base Optimization
template <typename FormattingPolicy>
class Document {
public:
    Document(std::string title, std::string body)
        : title_(std::move(title)), body_(std::move(body)) {}

    void render() const {
        // Direct call to policy method resolved at compile-time
        policy_.write_output(title_, body_);
    }

private:
    std::string title_;
    std::string body_;
    
    [[no_unique_address]] FormattingPolicy policy_;
};

int main() {
    // 1. Instantiate host class with PlainTextPolicy
    Document<PlainTextPolicy> plain_doc("System Report", "Status: Operational");
    plain_doc.render();

    std::cout << "---\n";

    // 2. Swap behavior simply by changing the template parameter
    Document<MarkdownPolicy> md_doc("System Report", "Status: Operational");
    md_doc.render();
}