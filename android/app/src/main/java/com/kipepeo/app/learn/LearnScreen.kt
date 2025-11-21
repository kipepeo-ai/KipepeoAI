package com.kipepeo.app.learn

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
fun LearnScreen(viewModel: LearnViewModel = viewModel()) {
    var question by remember { mutableStateOf("") }
    val result by viewModel.tutorResponse.collectAsState()
    val isLoading by viewModel.isLoading.collectAsState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black)
            .padding(16.dp)
    ) {
        Text(
            text = "Kipepeo Teacher",
            style = MaterialTheme.typography.headlineLarge,
            color = Color(0xFFFFC107)
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        TextField(
            value = question,
            onValueChange = { question = it },
            label = { Text("Ask a question (Biology, Math, etc.)") },
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
            onClick = { viewModel.askTutor("General", question) },
            enabled = !isLoading && question.isNotEmpty(),
            modifier = Modifier.fillMaxWidth(),
            colors = ButtonDefaults.buttonColors(containerColor = Color(0xFFFFC107))
        ) {
            Text("Ask Teacher", color = Color.Black)
        }
        
        Spacer(modifier = Modifier.height(32.dp))
        
        if (isLoading) {
            CircularProgressIndicator(color = Color(0xFFFFC107))
        }
        
        result?.let {
            Card(
                modifier = Modifier.fillMaxWidth(),
                colors = CardDefaults.cardColors(containerColor = Color(0xFF1A1A1A))
            ) {
                Column(modifier = Modifier.padding(16.dp)) {
                    Text("Answer:", color = Color(0xFFFFC107))
                    Text(it, color = Color.White)
                }
            }
        }
    }
}
