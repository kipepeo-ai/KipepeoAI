package com.kipepeo.app.health

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
fun HealthScreen(viewModel: HealthViewModel = viewModel()) {
    var symptoms by remember { mutableStateOf("") }
    val result by viewModel.diagnosisResult.collectAsState()
    val isLoading by viewModel.isLoading.collectAsState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black)
            .padding(16.dp)
    ) {
        Text(
            text = "Kipepeo Doctor",
            style = MaterialTheme.typography.headlineLarge,
            color = Color(0xFF4CAF50)
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        TextField(
            value = symptoms,
            onValueChange = { symptoms = it },
            label = { Text("Describe symptoms (e.g. headache, fever)") },
            modifier = Modifier.fillMaxWidth(),
            colors = TextFieldDefaults.colors(
                focusedContainerColor = Color(0xFF1A1A1A),
                unfocusedContainerColor = Color(0xFF1A1A1A),
                focusedTextColor = Color.White,
                unfocusedTextColor = Color.White
            )
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        Button(
            onClick = { viewModel.diagnose(symptoms) },
            enabled = !isLoading && symptoms.isNotEmpty(),
            modifier = Modifier.fillMaxWidth(),
            colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF4CAF50))
        ) {
            Text("Diagnose")
        }
        
        Spacer(modifier = Modifier.height(32.dp))
        
        if (isLoading) {
            CircularProgressIndicator(color = Color(0xFF4CAF50))
        }
        
        result?.let {
            val parts = it.split("|")
            if (parts.size >= 4) {
                DiagnosisCard(parts[0], parts[1], parts[2], parts[3])
            }
        }
    }
}

@Composable
fun DiagnosisCard(condition: String, confidence: String, recommendation: String, clinic: String) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(containerColor = Color(0xFF1A1A1A))
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text("Diagnosis: $condition", style = MaterialTheme.typography.titleLarge, color = Color.White)
            Text("Confidence: ${(confidence.toFloat() * 100).toInt()}%", color = Color.Gray)
            Spacer(modifier = Modifier.height(8.dp))
            Text("Recommendation:", color = Color(0xFF4CAF50))
            Text(recommendation, color = Color.White)
            Spacer(modifier = Modifier.height(8.dp))
            Text("Nearest Clinic:", color = Color(0xFF2196F3))
            Text(clinic, color = Color.White)
        }
    }
}
