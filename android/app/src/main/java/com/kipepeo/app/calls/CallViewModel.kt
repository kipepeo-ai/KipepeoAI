package com.kipepeo.app.calls

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch

enum class CallState {
    IDLE, CONNECTING, CONNECTED
}

data class Peer(val id: String, val name: String)

class CallViewModel : ViewModel() {
    private val _callState = MutableStateFlow(CallState.IDLE)
    val callState: StateFlow<CallState> = _callState

    private val _availablePeers = MutableStateFlow<List<Peer>>(emptyList())
    val availablePeers: StateFlow<List<Peer>> = _availablePeers

    init {
        // Mock peers
        _availablePeers.value = listOf(
            Peer("1", "Juma (Nearby)"),
            Peer("2", "Wanjiku (Nearby)")
        )
        
        // Initialize Native Engine
        initVoiceEngine()
    }

    fun startCall(peerId: String) {
        _callState.value = CallState.CONNECTING
        viewModelScope.launch {
            startCallNative(peerId)
            _callState.value = CallState.CONNECTED
        }
    }

    fun endCall() {
        endCallNative()
        _callState.value = CallState.IDLE
    }

    // JNI Methods
    private external fun initVoiceEngine()
    private external fun startCallNative(peerId: String)
    private external fun endCallNative()

    companion object {
        init {
            System.loadLibrary("kipepeo")
        }
    }
}
