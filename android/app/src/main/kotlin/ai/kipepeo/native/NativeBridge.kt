package ai.kipepeo.native

/**
 * JNI Bridge to Kipepeo native library
 * 
 * Provides Kotlin interface to C/C++ PLT hooking engine
 */
object NativeBridge {
    
    init {
        System.loadLibrary("kipepeo")
    }
    
    /**
     * Activate the Kipepeo Engine (PLT hooking)
     * 
     * @return true if successful, false otherwise
     */
    external fun activateKipepeoEngine(): Boolean
    
    /**
     * Deactivate the Kipepeo Engine
     */
    external fun deactivateKipepeoEngine()
    
    /**
     * Get LLM inference tokens per second
     * 
     * @return tokens/sec (0.0 if not running)
     */
    external fun getTokensPerSecond(): Float
    
    /**
     * Get total data saved in bytes
     * 
     * @return bytes saved via AV1 compression
     */
    external fun getDataSaved(): Long
    
    /**
     * Check if device has root privileges
     * 
     * @return true if root available
     */
    external fun isRootAvailable(): Boolean
    
    /**
     * Get current hook status as string
     * 
     * @return status string (e.g. "Active (Root Mode)")
     */
    external fun getHookStatus(): String
    
    /**
     * Get compression ratio
     * 
     * @return ratio (0.0-1.0)
     */
    external fun getCompressionRatio(): Double
    
    /**
     * Reset all statistics
     */
    external fun resetStats()
}
