package com.kipepeo.app.vision

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.unit.dp
import androidx.navigation.NavController

@Composable
fun VisionScreen(navController: NavController) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black)
            .padding(16.dp)
    ) {
        Text(
            text = "Kipepeo Vision",
            style = MaterialTheme.typography.headlineLarge,
            color = Color(0xFF00E5FF)
        )
        
        Spacer(modifier = Modifier.height(32.dp))
        
        VisionCard(
            title = "Farmer Mode",
            description = "Diagnose pests and crop diseases",
            icon = Icons.Default.Grass,
            color = Color(0xFF4CAF50),
            onClick = { navController.navigate("vision_farmer") }
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        VisionCard(
            title = "Image Generation",
            description = "Create art from Sheng prompts",
            icon = Icons.Default.Image,
            color = Color(0xFFE91E63),
            onClick = { navController.navigate("vision_gen") }
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        VisionCard(
            title = "Textbook Mode",
            description = "Scan questions & get answers",
            icon = Icons.Default.Book,
            color = Color(0xFFFFC107),
            onClick = { navController.navigate("vision_textbook") }
        )
    }
}

@Composable
fun VisionCard(
    title: String,
    description: String,
    icon: ImageVector,
    color: Color,
    onClick: () -> Unit
) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .height(120.dp)
            .clickable(onClick = onClick),
        shape = RoundedCornerShape(16.dp),
        colors = CardDefaults.cardColors(containerColor = Color(0xFF1A1A1A))
    ) {
        Row(
            modifier = Modifier
                .fillMaxSize()
                .padding(16.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Icon(
                imageVector = icon,
                contentDescription = null,
                tint = color,
                modifier = Modifier.size(48.dp)
            )
            Spacer(modifier = Modifier.width(16.dp))
            Column {
                Text(
                    text = title,
                    style = MaterialTheme.typography.titleLarge,
                    color = Color.White
                )
                Text(
                    text = description,
                    style = MaterialTheme.typography.bodyMedium,
                    color = Color.Gray
                )
            }
        }
    }
}
