package com.kipepeo.app.viral

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Share
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel

@Composable
fun ViralScreen(viewModel: ViralViewModel = viewModel()) {
    val referralCode by viewModel.referralCode.collectAsState()
    val stats by viewModel.impactStats.collectAsState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black)
            .padding(16.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Text(
            text = "Invite & Earn",
            style = MaterialTheme.typography.headlineLarge,
            color = Color(0xFFE040FB)
        )
        
        Spacer(modifier = Modifier.height(32.dp))
        
        Card(
            modifier = Modifier.fillMaxWidth(),
            colors = CardDefaults.cardColors(containerColor = Color(0xFF1A1A1A))
        ) {
            Column(
                modifier = Modifier.padding(24.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Text("Your Referral Code", color = Color.Gray)
                Spacer(modifier = Modifier.height(8.dp))
                Text(
                    text = referralCode,
                    style = MaterialTheme.typography.headlineMedium,
                    color = Color.White,
                    fontWeight = FontWeight.Bold
                )
                Spacer(modifier = Modifier.height(16.dp))
                Button(
                    onClick = { /* Share Intent */ },
                    colors = ButtonDefaults.buttonColors(containerColor = Color(0xFFE040FB))
                ) {
                    Icon(Icons.Default.Share, contentDescription = null)
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("Share Code")
                }
            }
        }
        
        Spacer(modifier = Modifier.height(32.dp))
        
        Text(
            text = "Your Impact",
            style = MaterialTheme.typography.titleLarge,
            color = Color.White
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        ImpactCard(
            title = "Data Saved",
            value = stats.dataSaved,
            color = Color(0xFF00E5FF)
        )
        Spacer(modifier = Modifier.height(8.dp))
        ImpactCard(
            title = "Money Saved",
            value = stats.moneySaved,
            color = Color(0xFF00E676)
        )
        Spacer(modifier = Modifier.height(8.dp))
        ImpactCard(
            title = "CO2 Offset (Trees)",
            value = stats.treesPlanted,
            color = Color(0xFFFFC107)
        )
    }
}

@Composable
fun ImpactCard(title: String, value: String, color: Color) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(containerColor = Color(0xFF1A1A1A))
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(title, color = Color.Gray)
            Text(
                text = value,
                style = MaterialTheme.typography.titleLarge,
                color = color,
                fontWeight = FontWeight.Bold
            )
        }
    }
}
