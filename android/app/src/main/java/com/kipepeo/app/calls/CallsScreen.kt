package com.kipepeo.app.calls

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
fun CallsScreen(viewModel: CallViewModel = viewModel()) {
    val callState by viewModel.callState.collectAsState()
    val peers by viewModel.availablePeers.collectAsState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black)
            .padding(16.dp)
    ) {
        Text(
            text = "Kipepeo Calls",
            style = MaterialTheme.typography.headlineLarge,
            color = Color(0xFF00E5FF) // Kipepeo Cyan
        )
        
        Spacer(modifier = Modifier.height(16.dp))

        if (callState == CallState.IDLE) {
            Text(
                text = "Available Peers (Wi-Fi Direct / BT)",
                style = MaterialTheme.typography.titleMedium,
                color = Color.White
            )
            LazyColumn {
                items(peers) { peer ->
                    PeerItem(peer) {
                        viewModel.startCall(peer.id)
                    }
                }
            }
        } else {
            ActiveCallUI(callState, viewModel::endCall)
        }
    }
}

@Composable
fun PeerItem(peer: Peer, onCall: () -> Unit) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 8.dp),
        colors = CardDefaults.cardColors(containerColor = Color(0xFF1A1A1A))
    ) {
        Row(
            modifier = Modifier
                .padding(16.dp)
                .fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(text = peer.name, color = Color.White)
            Button(
                onClick = onCall,
                colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF00E5FF))
            ) {
                Text("Call", color = Color.Black)
            }
        }
    }
}

@Composable
fun ActiveCallUI(state: CallState, onEndCall: () -> Unit) {
    Column(
        modifier = Modifier.fillMaxSize(),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        Text(
            text = if (state == CallState.CONNECTING) "Connecting..." else "Connected",
            style = MaterialTheme.typography.displaySmall,
            color = Color.White
        )
        Spacer(modifier = Modifier.height(32.dp))
        Button(
            onClick = onEndCall,
            colors = ButtonDefaults.buttonColors(containerColor = Color.Red)
        ) {
            Text("End Call", color = Color.White)
        }
    }
}
