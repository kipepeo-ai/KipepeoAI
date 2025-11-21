package com.kipepeo.app.mesh

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.coroutines.delay

data class Peer(
    val id: String,
    val name: String,
    val signalStrength: Int,
    val isConnected: Boolean
)

class MeshViewModel : ViewModel() {
    
    private val _peers = MutableStateFlow<List<Peer>>(emptyList())
    val peers: StateFlow<List<Peer>> = _peers
    
    private val _isScanning = MutableStateFlow(false)
    val isScanning: StateFlow<Boolean> = _isScanning

    init {
        initMeshEngine()
        startScan()
    }

    fun startScan() {
        viewModelScope.launch {
            _isScanning.value = true
            startDiscoveryNative()
            // Simulate scanning delay
            delay(2000) 
            refreshPeers()
            _isScanning.value = false
        }
    }
    
    private suspend fun refreshPeers() {
        val result = withContext(Dispatchers.IO) {
            getPeersNative()
        }
        _peers.value = parsePeers(result)
    }
    
    private fun parsePeers(raw: String): List<Peer> {
        // Mock parsing logic: ID|Name|Signal|Connected;...
        val list = mutableListOf<Peer>()
        val items = raw.split(";")
        for (item in items) {
            val parts = item.split("|")
            if (parts.size >= 4) {
                list.add(Peer(
                    parts[0], parts[1], parts[2].toIntOrNull() ?: 0, parts[3] == "1"
                ))
            }
        }
        return list
    }

    // JNI Methods
    private external fun initMeshEngine()
    private external fun startDiscoveryNative()
    private external fun getPeersNative(): String

    companion object {
        init {
            System.loadLibrary("kipepeo")
        }
    }
}
