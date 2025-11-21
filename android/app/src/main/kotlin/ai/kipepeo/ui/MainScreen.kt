package ai.kipepeo.ui

import androidx.compose.animation.core.*
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import ai.kipepeo.KipepeoState
import kotlin.math.roundToLong

@Composable
fun MainScreen(
    state: KipepeoState,
    onActivateEngine: () -> Unit,
    onDeactivateEngine: () -> Unit,
    onResetStats: () -> Unit
) {
    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Kipepeo AI") },
                colors = TopAppBarDefaults.topAppBarColors(
                    containerColor = MaterialTheme.colorScheme.primaryContainer,
                    titleContentColor = MaterialTheme.colorScheme.onPrimaryContainer
                )
            )
        }
    ) { paddingValues ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(paddingValues)
                .padding(24.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(24.dp)
        ) {
            // Root status indicator
            RootStatusCard(hasRoot = state.hasRoot)
            
            // Engine activation switch
            EngineControlCard(
                isActive = state.isEngineActive,
                hookStatus = state.hookStatus,
                onActivate = onActivateEngine,
                onDeactivate = onDeactivateEngine
            )
            
            // Metrics display
            MetricsCard(
                tokensPerSecond = state.tokensPerSecond,
                dataSavedBytes = state.dataSavedBytes,
                compressionRatio = state.compressionRatio
            )
            
            // Error display
            state.error?.let { error ->
                ErrorCard(error = error)
            }
            
            Spacer(modifier = Modifier.weight(1f))
            
            // Reset button
            Button(
                onClick = onResetStats,
                modifier = Modifier.fillMaxWidth(),
                colors = ButtonDefaults.buttonColors(
                    containerColor = MaterialTheme.colorScheme.error
                )
            ) {
                Text("Reset Statistics")
            }
        }
    }
}

@Composable
fun RootStatusCard(hasRoot: Boolean) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(
            containerColor = if (hasRoot) 
                MaterialTheme.colorScheme.tertiaryContainer 
            else 
                MaterialTheme.colorScheme.surfaceVariant
        )
    ) {
        Row(
            modifier = Modifier.padding(16.dp),
            horizontalArrangement = Arrangement.spacedBy(12.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = if (hasRoot) "🔓" else "🔒",
                fontSize = 24.sp
            )
            Column {
                Text(
                    text = if (hasRoot) "Root Access" else "Non-Root Mode",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Bold
                )
                Text(
                    text = if (hasRoot) 
                        "Full PLT hooking enabled" 
                    else 
                        "Limited functionality (VPN mode)",
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        }
    }
}

@Composable
fun EngineControlCard(
    isActive: Boolean,
    hookStatus: String,
    onActivate: () -> Unit,
    onDeactivate: () -> Unit
) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(
            containerColor = if (isActive)
                MaterialTheme.colorScheme.primaryContainer
            else
                MaterialTheme.colorScheme.surfaceVariant
        )
    ) {
        Column(
            modifier = Modifier.padding(20.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = "Kipepeo Engine",
                    style = MaterialTheme.typography.headlineSmall,
                    fontWeight = FontWeight.Bold
                )
                
                Switch(
                    checked = isActive,
                    onCheckedChange = { checked ->
                        if (checked) onActivate() else onDeactivate()
                    }
                )
            }
            
            Text(
                text = "Status: $hookStatus",
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
    }
}

@Composable
fun MetricsCard(
    tokensPerSecond: Float,
    dataSavedBytes: Long,
    compressionRatio: Double
) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.secondaryContainer
        )
    ) {
        Column(
            modifier = Modifier.padding(20.dp),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Text(
                text = "Performance Metrics",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold
            )
            
            // LLM Speed
            MetricRow(
                label = "LLM Speed",
                value = if (tokensPerSecond > 0) "${"%.1f".format(tokensPerSecond)} tokens/sec" else "Idle",
                emoji = "⚡"
            )
            
            HorizontalDivider()
            
            // Data Saved
            val dataSavedMB = dataSavedBytes / (1024.0 * 1024.0)
            MetricRow(
                label = "Data Saved",
                value = if (dataSavedBytes > 0) "${"%.2f".format(dataSavedMB)} MB" else "0 MB",
                emoji = "💾"
            )
            
            HorizontalDivider()
            
            // Compression Ratio
            val compressionPercent = ((1.0 - compressionRatio) * 100).roundToLong()
            MetricRow(
                label = "Compression",
                value = "$compressionPercent% reduction",
                emoji = "🗜️"
            )
        }
    }
}

@Composable
fun MetricRow(label: String, value: String, emoji: String) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Row(
            horizontalArrangement = Arrangement.spacedBy(8.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(text = emoji, fontSize = 20.sp)
            Text(
                text = label,
                style = MaterialTheme.typography.bodyLarge,
                color = MaterialTheme.colorScheme.onSecondaryContainer
            )
        }
        Text(
            text = value,
            style = MaterialTheme.typography.bodyLarge,
            fontWeight = FontWeight.Bold,
            color = MaterialTheme.colorScheme.onSecondaryContainer
        )
    }
}

@Composable
fun ErrorCard(error: String) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.errorContainer
        )
    ) {
        Row(
            modifier = Modifier.padding(16.dp),
            horizontalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(text = "⚠️", fontSize = 20.sp)
            Text(
                text = error,
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onErrorContainer
            )
        }
    }
}
