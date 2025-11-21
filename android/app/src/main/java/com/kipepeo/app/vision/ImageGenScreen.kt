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
fun ImageGenScreen(viewModel: VisionViewModel = viewModel()) {
    var prompt by remember { mutableStateOf("") }
    val isLoading by viewModel.isLoading.collectAsState()
    val generatedImage by viewModel.generatedImage.collectAsState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black)
            .padding(16.dp)
    ) {
        Text(
            text = "Generate Image",
            style = MaterialTheme.typography.headlineMedium,
            color = Color(0xFFE91E63)
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        TextField(
            value = prompt,
            onValueChange = { prompt = it },
            label = { Text("Enter prompt (Sheng/Swahili/English)") },
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
            onClick = { viewModel.generateImage(prompt) },
            enabled = !isLoading && prompt.isNotEmpty(),
            modifier = Modifier.fillMaxWidth(),
            colors = ButtonDefaults.buttonColors(containerColor = Color(0xFFE91E63))
        ) {
            Text("Generate")
        }
        
        Spacer(modifier = Modifier.height(32.dp))
        
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .aspectRatio(1f)
                .background(Color(0xFF1A1A1A)),
            contentAlignment = Alignment.Center
        ) {
            if (isLoading) {
                CircularProgressIndicator(color = Color(0xFFE91E63))
            } else if (generatedImage != null) {
                Text("Image Generated (Mock)", color = Color.White)
            } else {
                Text("No Image", color = Color.Gray)
            }
        }
    }
}
