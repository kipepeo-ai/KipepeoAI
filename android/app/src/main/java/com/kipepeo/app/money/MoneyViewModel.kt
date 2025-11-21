package com.kipepeo.app.money

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

data class Transaction(
    val id: String,
    val type: String,
    val amount: Double,
    val date: String,
    val isSuspicious: Boolean
)

class MoneyViewModel : ViewModel() {
    
    private val _transactions = MutableStateFlow<List<Transaction>>(emptyList())
    val transactions: StateFlow<List<Transaction>> = _transactions
    
    private val _isLoading = MutableStateFlow(false)
    val isLoading: StateFlow<Boolean> = _isLoading

    init {
        initFinanceEngine()
    }

    fun analyzeSms() {
        viewModelScope.launch {
            _isLoading.value = true
            val result = withContext(Dispatchers.IO) {
                analyzeTransactionsNative("mock_sms_data")
            }
            _transactions.value = parseTransactions(result)
            _isLoading.value = false
        }
    }
    
    private fun parseTransactions(raw: String): List<Transaction> {
        // Mock parsing logic: ID|Type|Amount|Date|Suspicious;...
        val list = mutableListOf<Transaction>()
        val items = raw.split(";")
        for (item in items) {
            val parts = item.split("|")
            if (parts.size >= 5) {
                list.add(Transaction(
                    parts[0], parts[1], parts[2].toDoubleOrNull() ?: 0.0, parts[3], parts[4] == "1"
                ))
            }
        }
        return list
    }

    // JNI Methods
    private external fun initFinanceEngine()
    private external fun analyzeTransactionsNative(smsData: String): String

    companion object {
        init {
            System.loadLibrary("kipepeo")
        }
    }
}
