package com.kipepeo.app.vision

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel

@Composable
fun FarmerModeScreen(viewModel: VisionViewModel = viewModel()) {
    val result by viewModel.descriptionResult.collectAsState()
    val isLoading by viewModel.isLoading.collectAsState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black)
            .padding(16.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .height(400.dp)
                .background(Color.DarkGray),
            contentAlignment = Alignment.Center
        ) {
            Text("Camera Preview Placeholder", color = Color.White)
        }
        
        Spacer(modifier = Modifier.height(32.dp))
        
        Button(
            onClick = { 
                // Mock taking a photo
                viewModel.describeImage(ByteArray(0)) 
            },
            enabled = !isLoading,
            colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF4CAF50))
        ) {
            Text("Analyze Crop")
        }
        
        Spacer(modifier = Modifier.height(16.dp))
        
        if (isLoading) {
            CircularProgressIndicator(color = Color(0xFF4CAF50))
        }
        
        result?.let {
            Text(
                text = it,
                style = MaterialTheme.typography.bodyLarge,
                color = Color.White
            )
        }
    }
}
