package com.kipepeo.app.ui

import androidx.compose.animation.*
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.kipepeo.app.ui.theme.*

enum class Language(val displayName: String, val nativeGreeting: String) {
    SWAHILI("Kiswahili", "Karibu Kipepeo! 🦋"),
    ENGLISH("English", "Welcome to Kipepeo! 🦋"),
    SHENG("Sheng", "Niaje! Karibu Kipepeo! 🦋")
}

@OptIn(ExperimentalAnimationApi::class)
@Composable
fun OnboardingScreen(
    onLanguageSelected: (Language) -> Unit = {}
) {
    var selectedLanguage by remember { mutableStateOf<Language?>(null) }
    
    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(KipepeoBlack)
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(24.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            // Logo/Title
            Text(
                text = "🦋",
                style = MaterialTheme.typography.displayLarge,
                modifier = Modifier.padding(bottom = 16.dp)
            )
            
            Text(
                text = "KIPEPEO",
                style = MaterialTheme.typography.displayMedium,
                color = KipepeoCyan,
                fontWeight = FontWeight.Bold,
                modifier = Modifier.padding(bottom = 8.dp)
            )
            
            Text(
                text = "Save Data. Chat Smarter. Learn Faster.",
                style = MaterialTheme.typography.bodyLarge,
                color = TextSecondary,
                textAlign = TextAlign.Center,
                modifier = Modifier.padding(bottom = 48.dp)
            )
            
            // Language selection
            Text(
                text = "Chagua lugha yako / Choose your language",
                style = MaterialTheme.typography.titleMedium,
                color = TextSecondary,
                modifier = Modifier.padding(bottom = 24.dp)
            )
            
            Language.values().forEach { language ->
                LanguageCard(
                    language = language,
                    isSelected = selectedLanguage == language,
                    onClick = {
                        selectedLanguage = language
                    }
                )
                
                Spacer(modifier = Modifier.height(16.dp))
            }
            
            Spacer(modifier = Modifier.height(32.dp))
            
            // Continue button
            AnimatedVisibility(
                visible = selectedLanguage != null,
                enter = fadeIn() + expandVertically(),
                exit = fadeOut() + shrinkVertically()
            ) {
                Button(
                    onClick = {
                        selectedLanguage?.let { onLanguageSelected(it) }
                    },
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(56.dp),
                    colors = ButtonDefaults.buttonColors(
                        containerColor = KipepeoCyan,
                        contentColor = KipepeoBlack
                    ),
                    shape = RoundedCornerShape(16.dp)
                ) {
                    Text(
                        text = when (selectedLanguage) {
                            Language.SWAHILI -> "Endelea"
                            Language.ENGLISH -> "Continue"
                            Language.SHENG -> "Twende!"
                            null -> "Continue"
                        },
                        style = MaterialTheme.typography.titleLarge,
                        fontWeight = FontWeight.Bold
                    )
                }
            }
        }
    }
}

@Composable
private fun LanguageCard(
    language: Language,
    isSelected: Boolean,
    onClick: () -> Unit
) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .height(80.dp)
            .clickable(onClick = onClick),
        shape = RoundedCornerShape(16.dp),
        colors = CardDefaults.cardColors(
            containerColor = if (isSelected) KipepeoGray else KipepeoBlackSoft
        ),
        elevation = CardDefaults.cardElevation(
            defaultElevation = if (isSelected) 8.dp else 2.dp
        )
    ) {
        Row(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 20.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Column {
                Text(
                    text = language.displayName,
                    style = MaterialTheme.typography.titleLarge,
                    color = if (isSelected) KipepeoCyan else TextPrimary,
                    fontWeight = if (isSelected) FontWeight.Bold else FontWeight.Normal
                )
                Text(
                    text = language.nativeGreeting,
                    style = MaterialTheme.typography.bodyMedium,
                    color = TextSecondary
                )
            }
            
            if (isSelected) {
                Text(
                    text = "✓",
                    style = MaterialTheme.typography.headlineLarge,
                    color = KipepeoCyan
                )
            }
        }
    }
}

@Preview(showBackground = true, backgroundColor = 0xFF000000)
@Composable
private fun OnboardingScreenPreview() {
    KipepeoTheme {
        OnboardingScreen()
    }
}
