package com.kipepeo.app.viral

import androidx.lifecycle.ViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow

class ViralViewModel : ViewModel() {
    
    private val _referralCode = MutableStateFlow("KIP-2025-X7Z")
    val referralCode: StateFlow<String> = _referralCode
    
    private val _impactStats = MutableStateFlow(
        ImpactStats(
            dataSaved = "2.5 GB",
            moneySaved = "KES 450",
            treesPlanted = "12" // Mock metric based on data saved
        )
    )
    val impactStats: StateFlow<ImpactStats> = _impactStats
}

data class ImpactStats(
    val dataSaved: String,
    val moneySaved: String,
    val treesPlanted: String
)
