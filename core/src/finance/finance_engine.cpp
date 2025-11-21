#include "finance_engine.h"
#include <iostream>

namespace kipepeo {
namespace finance {

FinanceEngine& FinanceEngine::instance() {
    static FinanceEngine instance;
    return instance;
}

void FinanceEngine::init() {
    std::cout << "[Kipepeo] FinanceEngine initialized (Kipepeo Finance 7B)" << std::endl;
}

std::vector<Transaction> FinanceEngine::analyzeTransactions(const std::string& smsData) {
    std::cout << "[Kipepeo] Analyzing SMS data for transactions..." << std::endl;
    // Mock transactions
    return {
        {"TX123", "M-PESA Sent", 500.0, "2025-11-21", false},
        {"TX124", "M-PESA Received", 2000.0, "2025-11-20", false},
        {"TX125", "Fuliza Loan", 100.0, "2025-11-19", true} // Suspicious/High Interest
    };
}

bool FinanceEngine::detectFraud(const std::string& transactionDetails) {
    return transactionDetails.find("win") != std::string::npos; // Simple mock check
}

} // namespace finance
} // namespace kipepeo
