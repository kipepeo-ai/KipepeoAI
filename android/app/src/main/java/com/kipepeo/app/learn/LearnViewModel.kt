package com.kipepeo.app.learn

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

class LearnViewModel : ViewModel() {
    
    private val _tutorResponse = MutableStateFlow<String?>(null)
    val tutorResponse: StateFlow<String?> = _tutorResponse
    
    private val _isLoading = MutableStateFlow(false)
    val isLoading: StateFlow<Boolean> = _isLoading

    init {
        initTutorEngine()
    }

    fun askTutor(subject: String, question: String) {
        viewModelScope.launch {
            _isLoading.value = true
            val result = withContext(Dispatchers.IO) {
                askTutorNative(subject, question)
            }
            _tutorResponse.value = result
            _isLoading.value = false
        }
    }

    // JNI Methods
    private external fun initTutorEngine()
    private external fun askTutorNative(subject: String, question: String): String

    companion object {
        init {
            System.loadLibrary("kipepeo")
        }
    }
}
