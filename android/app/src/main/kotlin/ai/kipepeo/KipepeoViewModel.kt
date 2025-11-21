package ai.kipepeo

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import ai.kipepeo.native.NativeBridge
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import android.util.Log

private const val TAG = "KipepeoViewModel"

data class KipepeoState(
    val isEngineActive: Boolean = false,
    val tokensPerSecond: Float = 0f,
    val dataSavedBytes: Long = 0L,
    val compressionRatio: Double = 1.0,
    val hookStatus: String = "Inactive",
    val hasRoot: Boolean = false,
    val error: String? = null
)

class KipepeoViewModel : ViewModel() {
    
    private val _state = MutableStateFlow(KipepeoState())
    val state: StateFlow<KipepeoState> = _state.asStateFlow()
    
    init {
        // Check root status on initialization
        checkRootStatus()
        
        // Start metrics update loop
        startMetricsUpdateLoop()
    }
    
    private fun checkRootStatus() {
        try {
            val hasRoot = NativeBridge.isRootAvailable()
            _state.value = _state.value.copy(hasRoot = hasRoot)
            Log.i(TAG, "Root status: $hasRoot")
        } catch (e: Exception) {
            Log.e(TAG, "Error checking root status", e)
            _state.value = _state.value.copy(error = "Failed to check root: ${e.message}")
        }
    }
    
    fun activateEngine() {
        viewModelScope.launch {
            try {
                Log.i(TAG, "Activating Kipepeo Engine...")
                val success = NativeBridge.activateKipepeoEngine()
                
                if (success) {
                    _state.value = _state.value.copy(
                        isEngineActive = true,
                        error = null
                    )
                    updateMetrics()
                    Log.i(TAG, "Engine activated successfully")
                } else {
                    _state.value = _state.value.copy(
                        isEngineActive = false,
                        error = "Failed to activate engine. Root required for full functionality."
                    )
                    Log.e(TAG, "Engine activation failed")
                }
            } catch (e: Exception) {
                Log.e(TAG, "Exception activating engine", e)
                _state.value = _state.value.copy(
                    isEngineActive = false,
                    error = "Error: ${e.message}"
                )
            }
        }
    }
    
    fun deactivateEngine() {
        viewModelScope.launch {
            try {
                Log.i(TAG, "Deactivating Kipepeo Engine...")
                NativeBridge.deactivateKipepeoEngine()
                _state.value = _state.value.copy(
                    isEngineActive = false,
                    hookStatus = "Inactive",
                    error = null
                )
                Log.i(TAG, "Engine deactivated")
            } catch (e: Exception) {
                Log.e(TAG, "Error deactivating engine", e)
                _state.value = _state.value.copy(error = "Deactivation error: ${e.message}")
            }
        }
    }
    
    fun resetStatistics() {
        viewModelScope.launch {
            try {
                NativeBridge.resetStats()
                updateMetrics()
                Log.i(TAG, "Statistics reset")
            } catch (e: Exception) {
                Log.e(TAG, "Error resetting stats", e)
            }
        }
    }
    
    private fun startMetricsUpdateLoop() {
        viewModelScope.launch {
            while (true) {
                if (_state.value.isEngineActive) {
                    updateMetrics()
                }
                delay(1000)  // Update every second
            }
        }
    }
    
    private fun updateMetrics() {
        try {
            val tokensPerSec = NativeBridge.getTokensPerSecond()
            val dataSaved = NativeBridge.getDataSaved()
            val compressionRatio = NativeBridge.getCompressionRatio()
            val hookStatus = NativeBridge.getHookStatus()
            
            _state.value = _state.value.copy(
                tokensPerSecond = tokensPerSec,
                dataSavedBytes = dataSaved,
                compressionRatio = compressionRatio,
                hookStatus = hookStatus
            )
        } catch (e: Exception) {
            Log.e(TAG, "Error updating metrics", e)
        }
    }
}
