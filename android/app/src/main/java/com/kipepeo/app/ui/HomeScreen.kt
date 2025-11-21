package com.kipepeo.app.ui

import androidx.compose.animation.core.*
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.kipepeo.app.ui.theme.FireOrange
import com.kipepeo.app.ui.theme.*

@Composable
fun HomeScreen(
    onNavigateToChat: () -> Unit = {},
    onNavigateToTranslation: () -> Unit = {},
    onNavigateToCalls: () -> Unit = {},
    onNavigateToVision: () -> Unit = {},
    onNavigateToKCSE: () -> Unit = {},
    dataSavedGB: Float = 1.87f
) {
    val scrollState = rememberScrollState()
    
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(KipepeoBlack)
            .verticalScroll(scrollState)
            .padding(24.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        // Header
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = "KIPEPEO 🦋",
                style = MaterialTheme.typography.headlineMedium,
                color = KipepeoCyan,
                fontWeight = FontWeight.Bold
            )
            
            IconButton(onClick = { /* Settings */ }) {
                Icon(
                    imageVector = Icons.Default.Settings,
                    contentDescription = "Settings",
                    tint = TextSecondary
                )
            }
        }
        
        Spacer(modifier = Modifier.height(32.dp))
        
        // Data Saved Card with Fire Animation
        DataSavedCard(dataSavedGB = dataSavedGB)
        
        Spacer(modifier = Modifier.height(32.dp))
        
        // Quick Actions
        Text(
            text = "Quick Actions",
            style = MaterialTheme.typography.titleLarge,
            color = TextPrimary,
            fontWeight = FontWeight.Bold,
            modifier = Modifier.fillMaxWidth()
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        QuickActionCard(
            icon = Icons.Default.Chat,
            title = "AI Chat",
            subtitle = "Voice & text with 34B LLM",
            gradient = Brush.horizontalGradient(
                colors = listOf(KipepeoCyan, KipepeoCyanDark)
            ),
            onClick = onNavigateToChat
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        QuickActionCard(
            icon = Icons.Default.Call,
            title = "Kipepeo Calls",
            subtitle = "Offline Voice & Translation",
            gradient = Brush.horizontalGradient(
                colors = listOf(Color(0xFF00E5FF), Color(0xFF00B8D4))
            ),
            onClick = onNavigateToCalls
        )
        
        Spacer(modifier = Modifier.height(16.dp))

        QuickActionCard(
            icon = Icons.Default.Visibility,
            title = "Kipepeo Vision",
            subtitle = "Farmer Mode & Image Gen",
            gradient = Brush.horizontalGradient(
                colors = listOf(Color(0xFFE91E63), Color(0xFFC2185B))
            ),
            onClick = onNavigateToVision
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        QuickActionCard(
            icon = Icons.Default.Translate,
            title = "Real-time Translation",
            subtitle = "Swahili ↔ English ↔ Sheng",
            gradient = Brush.horizontalGradient(
                colors = listOf(Color(0xFF9C27B0), Color(0xFF673AB7))
            ),
            onClick = onNavigateToTranslation
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        QuickActionCard(
            icon = Icons.Default.School,
            title = "KCSE 2026 Prep",
            subtitle = "AI tutor with curriculum PDFs",
            gradient = Brush.horizontalGradient(
                colors = listOf(SuccessGreen, Color(0xFF388E3C))
            ),
            onClick = onNavigateToKCSE
        )
        
        Spacer(modifier = Modifier.height(32.dp))
        
        // Stats Row
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceEvenly
        ) {
            StatCard(
                value = "30+",
                label = "Tokens/sec",
                icon = "⚡"
            )
            StatCard(
                value = "40%",
                label = "Data Saved",
                icon = "📊"
            )
            StatCard(
                value = "100%",
                label = "Offline",
                icon = "📱"
            )
        }
    }
}

@Composable
private fun DataSavedCard(dataSavedGB: Float) {
    // Animated fire effect
    val infiniteTransition = rememberInfiniteTransition(label = "fire")
    val scale by infiniteTransition.animateFloat(
        initialValue = 1f,
        targetValue = 1.2f,
        animationSpec = infiniteRepeatable(
            animation = tween(800, easing = FastOutSlowInEasing),
            repeatMode = RepeatMode.Reverse
        ),
        label = "fire_scale"
    )
    val alpha by infiniteTransition.animateFloat(
        initialValue = 0.7f,
        targetValue = 1f,
        animationSpec = infiniteRepeatable(
            animation = tween(600, easing = LinearEasing),
            repeatMode = RepeatMode.Reverse
        ),
        label = "fire_alpha"
    )
    
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .height(200.dp),
        shape = RoundedCornerShape(24.dp),
        colors = CardDefaults.cardColors(
            containerColor = KipepeoGray
        ),
        elevation = CardDefaults.cardElevation(
            defaultElevation = 8.dp
        )
    ) {
        Box(
            modifier = Modifier.fillMaxSize(),
            contentAlignment = Alignment.Center
        ) {
            Column(
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Text(
                    text = "Data saved today",
                    style = MaterialTheme.typography.titleMedium,
                    color = TextSecondary
                )
                
                Spacer(modifier = Modifier.height(8.dp))
                
                Row(
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(
                        text = String.format("%.2f GB", dataSavedGB),
                        style = MaterialTheme.typography.displayLarge,
                        color = KipepeoCyan,
                        fontWeight = FontWeight.Bold,
                        fontSize = 48.sp
                    )
                    
                    Spacer(modifier = Modifier.width(16.dp))
                    
                    // Fire emoji with animation
                    Text(
                        text = "🔥",
                        fontSize = 48.sp,
                        modifier = Modifier
                            .scale(scale)
                            .alpha(alpha)
                    )
                }
                
                Spacer(modifier = Modifier.height(8.dp))
                
                Text(
                    text = "Keep it up! You're saving the planet 🌍",
                    style = MaterialTheme.typography.bodyMedium,
                    color = TextTertiary,
                    textAlign = TextAlign.Center
                )
            }
        }
    }
}

@Composable
private fun QuickActionCard(
    icon: ImageVector,
    title: String,
    subtitle: String,
    gradient: Brush,
    onClick: () -> Unit
) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .height(100.dp)
            .clickable(onClick = onClick),
        shape = RoundedCornerShape(20.dp),
        colors = CardDefaults.cardColors(
            containerColor = Color.Transparent
        )
    ) {
        Box(
            modifier = Modifier
                .fillMaxSize()
                .background(gradient)
                .padding(20.dp)
        ) {
            Row(
                verticalAlignment = Alignment.CenterVertically,
                modifier = Modifier.fillMaxWidth()
            ) {
                Box(
                    modifier = Modifier
                        .size(50.dp)
                        .background(
                            color = Color.White.copy(alpha = 0.2f),
                            shape = CircleShape
                        ),
                    contentAlignment = Alignment.Center
                ) {
                    Icon(
                        imageVector = icon,
                        contentDescription = title,
                        tint = Color.White,
                        modifier = Modifier.size(28.dp)
                    )
                }
                
                Spacer(modifier = Modifier.width(16.dp))
                
                Column {
                    Text(
                        text = title,
                        style = MaterialTheme.typography.titleLarge,
                        color = Color.White,
                        fontWeight = FontWeight.Bold
                    )
                    Text(
                        text = subtitle,
                        style = MaterialTheme.typography.bodyMedium,
                        color = Color.White.copy(alpha = 0.9f)
                    )
                }
            }
        }
    }
}

@Composable
private fun StatCard(
    value: String,
    label: String,
    icon: String
) {
    Card(
        modifier = Modifier.size(100.dp),
        shape = RoundedCornerShape(16.dp),
        colors = CardDefaults.cardColors(
            containerColor = KipepeoBlackSoft
        )
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(12.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            Text(
                text = icon,
                fontSize = 24.sp
            )
            Spacer(modifier = Modifier.height(4.dp))
            Text(
                text = value,
                style = MaterialTheme.typography.titleLarge,
                color = KipepeoCyan,
                fontWeight = FontWeight.Bold
            )
            Text(
                text = label,
                style = MaterialTheme.typography.bodySmall,
                color = TextTertiary,
                textAlign = TextAlign.Center
            )
        }
    }
}

@Preview(showBackground = true, backgroundColor = 0xFF000000)
@Composable
private fun HomeScreenPreview() {
    KipepeoTheme {
        HomeScreen()
    }
}
