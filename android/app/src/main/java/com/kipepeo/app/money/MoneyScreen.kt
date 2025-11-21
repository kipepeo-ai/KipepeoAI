package com.kipepeo.app.money

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel

@Composable
fun MoneyScreen(viewModel: MoneyViewModel = viewModel()) {
    val transactions by viewModel.transactions.collectAsState()
    val isLoading by viewModel.isLoading.collectAsState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black)
            .padding(16.dp)
    ) {
        Text(
            text = "Kipepeo Money",
            style = MaterialTheme.typography.headlineLarge,
            color = Color(0xFF00E676)
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        Card(
            modifier = Modifier.fillMaxWidth(),
            colors = CardDefaults.cardColors(containerColor = Color(0xFF1A1A1A))
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Text("Total Balance", color = Color.Gray)
                Text("KES 2,500.00", style = MaterialTheme.typography.headlineMedium, color = Color.White)
            }
        }
        
        Spacer(modifier = Modifier.height(16.dp))
        
        Button(
            onClick = { viewModel.analyzeSms() },
            enabled = !isLoading,
            modifier = Modifier.fillMaxWidth(),
            colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF00E676))
        ) {
            Text("Analyze M-Pesa SMS")
        }
        
        Spacer(modifier = Modifier.height(16.dp))
        
        if (isLoading) {
            CircularProgressIndicator(color = Color(0xFF00E676))
        }
        
        LazyColumn {
            items(transactions) { tx ->
                TransactionItem(tx)
                Spacer(modifier = Modifier.height(8.dp))
            }
        }
    }
}

@Composable
fun TransactionItem(tx: Transaction) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(containerColor = if (tx.isSuspicious) Color(0xFF3E2723) else Color(0xFF1A1A1A))
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column {
                Text(tx.type, color = Color.White, style = MaterialTheme.typography.bodyLarge)
                Text(tx.date, color = Color.Gray, style = MaterialTheme.typography.bodySmall)
                if (tx.isSuspicious) {
                    Text("Suspicious Activity", color = Color.Red, style = MaterialTheme.typography.labelSmall)
                }
            }
            Text(
                "KES ${tx.amount}",
                color = if (tx.type.contains("Received")) Color(0xFF00E676) else Color.White,
                style = MaterialTheme.typography.titleMedium
            )
        }
    }
}
