package com.kipepeo.app.health

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

class HealthViewModel : ViewModel() {
    
    private val _diagnosisResult = MutableStateFlow<String?>(null)
    val diagnosisResult: StateFlow<String?> = _diagnosisResult
    
    private val _isLoading = MutableStateFlow(false)
    val isLoading: StateFlow<Boolean> = _isLoading

    init {
        initHealthEngine()
    }

    fun diagnose(symptoms: String) {
        viewModelScope.launch {
            _isLoading.value = true
            val result = withContext(Dispatchers.IO) {
                diagnoseNative(symptoms)
            }
            _diagnosisResult.value = result
            _isLoading.value = false
        }
    }
    
    fun clearResult() {
        _diagnosisResult.value = null
    }

    // JNI Methods
    private external fun initHealthEngine()
    private external fun diagnoseNative(symptoms: String): String

    companion object {
        init {
            System.loadLibrary("kipepeo")
        }
    }
}
