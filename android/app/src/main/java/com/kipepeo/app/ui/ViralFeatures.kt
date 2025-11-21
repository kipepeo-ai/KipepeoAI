package com.kipepeo.app.ui

import androidx.compose.animation.core.*
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.window.Dialog
import com.kipepeo.app.ui.theme.*

// Share Card Composable for viral sharing
@Composable
fun ShareCard(
    dataSavedGB: Float = 12.0f,
    onShare: (Platform) -> Unit = {}
) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .padding(16.dp),
        shape = RoundedCornerShape(24.dp),
        colors = CardDefaults.cardColors(
            containerColor = KipepeoBlack
        ),
        elevation = CardDefaults.cardElevation(
            defaultElevation = 12.dp
        )
    ) {
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .background(
                    Brush.verticalGradient(
                        colors = listOf(
                            KipepeoCyan.copy(alpha = 0.3f),
                            KipepeoBlack
                        )
                    )
                )
                .border(
                    width = 2.dp,
                    color = KipepeoCyan,
                    shape = RoundedCornerShape(24.dp)
                )
                .padding(24.dp)
        ) {
            Column(
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Text(
                    text = "🦋",
                    fontSize = 48.sp
                )
                
                Spacer(modifier = Modifier.height(16.dp))
                
                Text(
                    text = "I saved ${String.format("%.1f", dataSavedGB)} GB this month with Kipepeo! 🔥",
                    style = MaterialTheme.typography.headlineSmall,
                    color = TextPrimary,
                    fontWeight = FontWeight.Bold,
                    textAlign = TextAlign.Center
                )
                
                Spacer(modifier = Modifier.height(8.dp))
                
                Text(
                    text = "Join me and save data while chatting with AI offline!",
                    style = MaterialTheme.typography.bodyLarge,
                    color = TextSecondary,
                    textAlign = TextAlign.Center
                )
                
                Spacer(modifier = Modifier.height(24.dp))
                
                // Share buttons
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceEvenly
                ) {
                    SharePlatformButton(
                        platform = Platform.WHATSAPP,
                        icon = "💬",
                        onClick = { onShare(Platform.WHATSAPP) }
                    )
                    
                    SharePlatformButton(
                        platform = Platform.TIKTOK,
                        icon = "🎵",
                        onClick = { onShare(Platform.TIKTOK) }
                    )
                    
                    SharePlatformButton(
                        platform = Platform.TWITTER,
                        icon = "🐦",
                        onClick = { onShare(Platform.TWITTER) }
                    )
                    
                    SharePlatformButton(
                        platform = Platform.OTHER,
                        icon = "📤",
                        onClick = { onShare(Platform.OTHER) }
                    )
                }
                
                Spacer(modifier = Modifier.height(16.dp))
                
                Text(
                    text = "#KipepeoAI #SaveData #OfflineAI",
                    style = MaterialTheme.typography.bodySmall,
                    color = KipepeoCyan
                )
            }
        }
    }
}

@Composable
private fun SharePlatformButton(
    platform: Platform,
    icon: String,
    onClick: () -> Unit
) {
    Button(
        onClick = onClick,
        modifier = Modifier.size(70.dp),
        colors = ButtonDefaults.buttonColors(
            containerColor = when (platform) {
                Platform.WHATSAPP -> WhatsAppGreen
                Platform.TIKTOK -> Color(0xFF000000)
                Platform.TWITTER -> Color(0xFF1DA1F2)
                Platform.OTHER -> KipepeoGray
            }
        ),
        shape = RoundedCornerShape(16.dp),
        contentPadding = PaddingValues(0.dp)
    ) {
        Text(
            text = icon,
            fontSize = 28.sp
        )
    }
}

enum class Platform {
    WHATSAPP, TIKTOK, TWITTER, OTHER
}

// Referral Dialog
@Composable
fun ReferralDialog(
    referralCount: Int = 2,
    onDismiss: () -> Unit = {},
    onInviteFriends: () -> Unit = {}
) {
    Dialog(onDismissRequest = onDismiss) {
        Card(
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(24.dp),
            colors = CardDefaults.cardColors(
                containerColor = KipepeoBlackSoft
            )
        ) {
            Column(
                modifier = Modifier.padding(24.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Text(
                    text = "🚀",
                    fontSize = 48.sp
                )
                
                Spacer(modifier = Modifier.height(16.dp))
                
                Text(
                    text = "Unlock 70B Model!",
                    style = MaterialTheme.typography.headlineMedium,
                    color = KipepeoCyan,
                    fontWeight = FontWeight.Bold
                )
                
                Spacer(modifier = Modifier.height(8.dp))
                
                Text(
                    text = "Invite 5 friends to unlock the 70B model free forever!",
                    style = MaterialTheme.typography.bodyLarge,
                    color = TextSecondary,
                    textAlign = TextAlign.Center
                )
                
                Spacer(modifier = Modifier.height(24.dp))
                
                // Progress indicator
                Column(
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween
                    ) {
                        Text(
                            text = "$referralCount / 5 friends",
                            style = MaterialTheme.typography.titleMedium,
                            color = TextPrimary,
                            fontWeight = FontWeight.Bold
                        )
                        Text(
                            text = "${(referralCount * 20)}%",
                            style = MaterialTheme.typography.titleMedium,
                            color = KipepeoCyan,
                            fontWeight = FontWeight.Bold
                        )
                    }
                    
                    Spacer(modifier = Modifier.height(8.dp))
                    
                    LinearProgressIndicator(
                        progress = referralCount / 5f,
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(12.dp)
                            .clip(RoundedCornerShape(6.dp)),
                        color = KipepeoCyan,
                        trackColor = KipepeoGray
                    )
                }
                
                Spacer(modifier = Modifier.height(24.dp))
                
                // Referral rewards list
                Column(
                    modifier = Modifier.fillMaxWidth(),
                    verticalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    ReferralMilestone(
                        count = 1,
                        reward = "Unlock voice mode",
                        isUnlocked = referralCount >= 1
                    )
                    ReferralMilestone(
                        count = 3,
                        reward = "50B model access",
                        isUnlocked = referralCount >= 3
                    )
                    ReferralMilestone(
                        count = 5,
                        reward = "70B model FOREVER! 🔥",
                        isUnlocked = referralCount >= 5
                    )
                }
                
                Spacer(modifier = Modifier.height(24.dp))
                
                Button(
                    onClick = onInviteFriends,
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(56.dp),
                    colors = ButtonDefaults.buttonColors(
                        containerColor = KipepeoCyan,
                        contentColor = KipepeoBlack
                    ),
                    shape = RoundedCornerShape(16.dp)
                ) {
                    Icon(
                        imageVector = Icons.Default.Share,
                        contentDescription = null,
                        modifier = Modifier.size(24.dp)
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text(
                        text = "Invite Friends",
                        style = MaterialTheme.typography.titleLarge,
                        fontWeight = FontWeight.Bold
                    )
                }
                
                Spacer(modifier = Modifier.height(12.dp))
                
                TextButton(onClick = onDismiss) {
                    Text("Later", color = TextSecondary)
                }
            }
        }
    }
}

@Composable
private fun ReferralMilestone(
    count: Int,
    reward: String,
    isUnlocked: Boolean
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Box(
            modifier = Modifier
                .size(32.dp)
                .background(
                    color = if (isUnlocked) SuccessGreen else KipepeoGray,
                    shape = RoundedCornerShape(8.dp)
                ),
            contentAlignment = Alignment.Center
        ) {
            if (isUnlocked) {
                Text(text = "✓", color = Color.White, fontWeight = FontWeight.Bold)
            } else {
                Text(text = "$count", color = TextSecondary, fontWeight = FontWeight.Bold)
            }
        }
        
        Spacer(modifier = Modifier.width(12.dp))
        
        Text(
            text = reward,
            style = MaterialTheme.typography.bodyMedium,
            color = if (isUnlocked) TextPrimary else TextTertiary
        )
    }
}

// M-Pesa Payment Dialog
@Composable
fun MPesaPaymentDialog(
    onDismiss: () -> Unit = {},
    onPayNow: (String) -> Unit = {}
) {
    var phoneNumber by remember { mutableStateOf("") }
    var isProcessing by remember { mutableStateOf(false) }
    
    Dialog(onDismissRequest = onDismiss) {
        Card(
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(24.dp),
            colors = CardDefaults.cardColors(
                containerColor = KipepeoBlackSoft
            )
        ) {
            Column(
                modifier = Modifier.padding(24.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                // M-Pesa logo simulation
                Box(
                    modifier = Modifier
                        .size(80.dp)
                        .background(MPesaGreen, RoundedCornerShape(16.dp)),
                    contentAlignment = Alignment.Center
                ) {
                    Text(
                        text = "M-PESA",
                        color = Color.White,
                        fontWeight = FontWeight.Bold,
                        fontSize = 16.sp
                    )
                }
                
                Spacer(modifier = Modifier.height(16.dp))
                
                Text(
                    text = "Go Premium!",
                    style = MaterialTheme.typography.headlineMedium,
                    color = TextPrimary,
                    fontWeight = FontWeight.Bold
                )
                
                Spacer(modifier = Modifier.height(8.dp))
                
                Text(
                    text = "KSh 99/month",
                    style = MaterialTheme.typography.headlineSmall,
                    color = KipepeoCyan,
                    fontWeight = FontWeight.Bold
                )
                
                Spacer(modifier = Modifier.height(24.dp))
                
                // Premium features
                Column(
                    modifier = Modifier.fillMaxWidth(),
                    verticalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    PremiumFeature("✓", "Unlimited AI conversations")
                    PremiumFeature("✓", "70B model access")
                    PremiumFeature("✓", "Priority data compression")
                    PremiumFeature("✓", "Ad-free experience")
                }
                
                Spacer(modifier = Modifier.height(24.dp))
                
                // Phone number input
                OutlinedTextField(
                    value = phoneNumber,
                    onValueChange = { phoneNumber = it },
                    modifier = Modifier.fillMaxWidth(),
                    label = { Text("M-Pesa Phone Number", color = TextSecondary) },
                    placeholder = { Text("0712345678", color = TextTertiary) },
                    shape = RoundedCornerShape(16.dp),
                    colors = OutlinedTextFieldDefaults.colors(
                        focusedContainerColor = KipepeoGray,
                        unfocusedContainerColor = KipepeoGray,
                        focusedBorderColor = MPesaGreen,
                        unfocusedBorderColor = KipepeoGrayLight,
                        focusedTextColor = TextPrimary,
                        unfocusedTextColor = TextPrimary,
                        cursorColor = MPesaGreen
                    ),
                    singleLine = true,
                    enabled = !isProcessing
                )
                
                Spacer(modifier = Modifier.height(24.dp))
                
                Button(
                    onClick = {
                        isProcessing = true
                        onPayNow(phoneNumber)
                    },
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(56.dp),
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MPesaGreen,
                        contentColor = Color.White
                    ),
                    shape = RoundedCornerShape(16.dp),
                    enabled = phoneNumber.isNotBlank() && !isProcessing
                ) {
                    if (isProcessing) {
                        CircularProgressIndicator(
                            modifier = Modifier.size(24.dp),
                            color = Color.White,
                            strokeWidth = 2.dp
                        )
                        Spacer(modifier = Modifier.width(8.dp))
                        Text("Processing STK Push...")
                    } else {
                        Text(
                            text = "Pay KSh 99",
                            style = MaterialTheme.typography.titleLarge,
                            fontWeight = FontWeight.Bold
                        )
                    }
                }
                
                Spacer(modifier = Modifier.height(12.dp))
                
                TextButton(onClick = onDismiss) {
                    Text("Cancel", color = TextSecondary)
                }
            }
        }
    }
}

@Composable
private fun PremiumFeature(icon: String, text: String) {
    Row(
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = icon,
            color = SuccessGreen,
            fontSize = 20.sp,
            fontWeight = FontWeight.Bold
        )
        Spacer(modifier = Modifier.width(12.dp))
        Text(
            text = text,
            style = MaterialTheme.typography.bodyLarge,
            color = TextPrimary
        )
    }
}

// Preview composables
@Preview(showBackground = true, backgroundColor = 0xFF000000)
@Composable
private fun ShareCardPreview() {
    KipepeoTheme {
        ShareCard()
    }
}

@Preview(showBackground = true, backgroundColor = 0xFF000000)
@Composable
private fun ReferralDialogPreview() {
    KipepeoTheme {
        ReferralDialog()
    }
}

@Preview(showBackground = true, backgroundColor = 0xFF000000)
@Composable
private fun MPesaPaymentDialogPreview() {
    KipepeoTheme {
        MPesaPaymentDialog()
    }
}
