package com.kipepeo.app.mesh

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material.icons.filled.Wifi
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel

@Composable
fun MeshScreen(viewModel: MeshViewModel = viewModel()) {
    val peers by viewModel.peers.collectAsState()
    val isScanning by viewModel.isScanning.collectAsState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black)
            .padding(16.dp)
    ) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = "Kipepeo Mesh",
                style = MaterialTheme.typography.headlineLarge,
                color = Color(0xFF2979FF)
            )
            IconButton(onClick = { viewModel.startScan() }, enabled = !isScanning) {
                Icon(Icons.Default.Refresh, contentDescription = "Scan", tint = Color.White)
            }
        }
        
        Spacer(modifier = Modifier.height(8.dp))
        Text("Offline Network (Wi-Fi Direct + BT)", color = Color.Gray)
        Spacer(modifier = Modifier.height(24.dp))
        
        if (isScanning) {
            LinearProgressIndicator(
                modifier = Modifier.fillMaxWidth(),
                color = Color(0xFF2979FF)
            )
            Spacer(modifier = Modifier.height(16.dp))
        }
        
        LazyColumn {
            items(peers) { peer ->
                PeerItem(peer)
                Spacer(modifier = Modifier.height(8.dp))
            }
        }
    }
}

@Composable
fun PeerItem(peer: Peer) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(containerColor = Color(0xFF1A1A1A))
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Icon(
                imageVector = Icons.Default.Wifi,
                contentDescription = null,
                tint = if (peer.isConnected) Color(0xFF00E676) else Color.Gray
            )
            Spacer(modifier = Modifier.width(16.dp))
            Column(modifier = Modifier.weight(1f)) {
                Text(peer.name, color = Color.White, style = MaterialTheme.typography.titleMedium)
                Text("Signal: ${peer.signalStrength}%", color = Color.Gray, style = MaterialTheme.typography.bodySmall)
            }
            if (peer.isConnected) {
                Text("Connected", color = Color(0xFF00E676), style = MaterialTheme.typography.labelSmall)
            }
        }
    }
}
