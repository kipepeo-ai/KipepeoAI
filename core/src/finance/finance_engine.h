#pragma once

#include <string>
#include <vector>

namespace kipepeo {
namespace finance {

struct Transaction {
    std::string id;
    std::string type;
    double amount;
    std::string date;
    bool isSuspicious;
};

class FinanceEngine {
public:
    static FinanceEngine& instance();

    void init();
    
    std::vector<Transaction> analyzeTransactions(const std::string& smsData);
    bool detectFraud(const std::string& transactionDetails);

private:
    FinanceEngine() = default;
    ~FinanceEngine() = default;
};

} // namespace finance
} // namespace kipepeo
