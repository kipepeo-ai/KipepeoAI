package com.kipepeo.app.vision

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

class VisionViewModel : ViewModel() {
    
    private val _descriptionResult = MutableStateFlow<String?>(null)
    val descriptionResult: StateFlow<String?> = _descriptionResult
    
    private val _generatedImage = MutableStateFlow<ByteArray?>(null)
    val generatedImage: StateFlow<ByteArray?> = _generatedImage
    
    private val _isLoading = MutableStateFlow(false)
    val isLoading: StateFlow<Boolean> = _isLoading

    init {
        initVisionEngine()
    }

    fun describeImage(imageData: ByteArray) {
        viewModelScope.launch {
            _isLoading.value = true
            val result = withContext(Dispatchers.IO) {
                describeImageNative(imageData)
            }
            _descriptionResult.value = result
            _isLoading.value = false
        }
    }

    fun generateImage(prompt: String) {
        viewModelScope.launch {
            _isLoading.value = true
            val result = withContext(Dispatchers.IO) {
                generateImageNative(prompt)
            }
            _generatedImage.value = result
            _isLoading.value = false
        }
    }
    
    fun clearResults() {
        _descriptionResult.value = null
        _generatedImage.value = null
    }

    // JNI Methods
    private external fun initVisionEngine()
    private external fun describeImageNative(imageData: ByteArray): String
    private external fun generateImageNative(prompt: String): ByteArray

    companion object {
        init {
            System.loadLibrary("kipepeo")
        }
    }
}
