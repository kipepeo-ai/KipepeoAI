package com.kipepeo.app.ui

import androidx.compose.animation.core.*
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.kipepeo.app.ui.theme.*
import kotlinx.coroutines.launch

data class ChatMessage(
    val text: String,
    val isUser: Boolean,
    val timestamp: Long = System.currentTimeMillis()
)

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ChatScreen(
    onBack: () -> Unit = {}
) {
    var messageText by remember { mutableStateOf("") }
    var messages by remember { 
        mutableStateOf(
            listOf(
                ChatMessage("Habari! I'm your Kipepeo AI assistant. How can I help you today? 🦋", false),
                ChatMessage("Niaje! Ungependa kujua nini?", false)
            )
        )
    }
    var isRecording by remember { mutableStateOf(false) }
    var isProcessing by remember { mutableStateOf(false) }
    
    val listState = rememberLazyListState()
    val coroutineScope = rememberCoroutineScope()
    
    Scaffold(
        topBar = {
            TopAppBar(
                title = {
                    Column {
                        Text(
                            text = "AI Chat 🤖",
                            style = MaterialTheme.typography.titleLarge,
                            fontWeight = FontWeight.Bold
                        )
                        Text(
                            text = "34B LLM • Offline",
                            style = MaterialTheme.typography.bodySmall,
                            color = SuccessGreen
                        )
                    }
                },
                navigationIcon = {
                    IconButton(onClick = onBack) {
                        Icon(
                            imageVector = Icons.Default.ArrowBack,
                            contentDescription = "Back",
                            tint = TextPrimary
                        )
                    }
                },
                colors = TopAppBarDefaults.topAppBarColors(
                    containerColor = KipepeoBlack,
                    titleContentColor = TextPrimary
                )
            )
        },
        containerColor = KipepeoBlack
    ) { padding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
        ) {
            // Messages list
            LazyColumn(
                modifier = Modifier
                    .weight(1f)
                    .fillMaxWidth()
                    .padding(horizontal = 16.dp),
                state = listState,
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                items(messages) { message ->
                    ChatMessageBubble(message)
                }
                
                // Processing indicator
                if (isProcessing) {
                    item {
                        ProcessingIndicator()
                    }
                }
            }
            
            // Input area
            ChatInputArea(
                messageText = messageText,
                onMessageTextChanged = { messageText = it },
                isRecording = isRecording,
                onSendClick = {
                    if (messageText.isNotBlank()) {
                        messages = messages + ChatMessage(messageText, true)
                        messageText = ""
                        isProcessing = true
                        
                        // Scroll to bottom
                        coroutineScope.launch {
                            listState.animateScrollToItem(messages.size)
                        }
                        
                        // Simulate AI response
                        coroutineScope.launch {
                            kotlinx.coroutines.delay(1500)
                            messages = messages + ChatMessage(
                                "Asante for your question! Let me help you with that. (This is a demo response)",
                                false
                            )
                            isProcessing = false
                        }
                    }
                },
                onVoiceClick = {
                    isRecording = !isRecording
                }
            )
        }
    }
}

@Composable
private fun ChatMessageBubble(message: ChatMessage) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = if (message.isUser) Arrangement.End else Arrangement.Start
    ) {
        Card(
            modifier = Modifier.widthIn(max = 280.dp),
            shape = RoundedCornerShape(
                topStart = 20.dp,
                topEnd = 20.dp,
                bottomStart = if (message.isUser) 20.dp else 4.dp,
                bottomEnd = if (message.isUser) 4.dp else 20.dp
            ),
            colors = CardDefaults.cardColors(
                containerColor = if (message.isUser) KipepeoCyan else KipepeoGray
            )
        ) {
            Text(
                text = message.text,
                modifier = Modifier.padding(16.dp),
                style = MaterialTheme.typography.bodyLarge,
                color = if (message.isUser) KipepeoBlack else TextPrimary
            )
        }
    }
}

@Composable
private fun ProcessingIndicator() {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.Start
    ) {
        Card(
            modifier = Modifier.width(80.dp),
            shape = RoundedCornerShape(20.dp),
            colors = CardDefaults.cardColors(
                containerColor = KipepeoGray
            )
        ) {
            Row(
                modifier = Modifier.padding(16.dp),
                horizontalArrangement = Arrangement.spacedBy(4.dp)
            ) {
                repeat(3) { index ->
                    val infiniteTransition = rememberInfiniteTransition(label = "dot_$index")
                    val scale by infiniteTransition.animateFloat(
                        initialValue = 0.5f,
                        targetValue = 1f,
                        animationSpec = infiniteRepeatable(
                            animation = tween(600, delayMillis = index * 200),
                            repeatMode = RepeatMode.Reverse
                        ),
                        label = "dot_scale_$index"
                    )
                    
                    Box(
                        modifier = Modifier
                            .size(8.dp)
                            .scale(scale)
                            .background(KipepeoCyan, CircleShape)
                    )
                }
            }
        }
    }
}

@Composable
private fun ChatInputArea(
    messageText: String,
    onMessageTextChanged: (String) -> Unit,
    isRecording: Boolean,
    onSendClick: () -> Unit,
    onVoiceClick: () -> Unit
) {
    Surface(
        modifier = Modifier.fillMaxWidth(),
        color = KipepeoBlackSoft,
        shadowElevation = 8.dp
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalAlignment = Alignment.Bottom,
            horizontalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            // Voice button
            VoiceButton(
                isRecording = isRecording,
                onClick = onVoiceClick
            )
            
            // Text field
            OutlinedTextField(
                value = messageText,
                onValueChange = onMessageTextChanged,
                modifier = Modifier.weight(1f),
                placeholder = {
                    Text(
                        text = "Type or speak... (Swahili/English/Sheng)",
                        color = TextTertiary,
                        style = MaterialTheme.typography.bodyMedium
                    )
                },
                shape = RoundedCornerShape(24.dp),
                colors = OutlinedTextFieldDefaults.colors(
                    focusedContainerColor = KipepeoGray,
                    unfocusedContainerColor = KipepeoGray,
                    focusedBorderColor = KipepeoCyan,
                    unfocusedBorderColor = KipepeoGrayLight,
                    focusedTextColor = TextPrimary,
                    unfocusedTextColor = TextPrimary,
                    cursorColor = KipepeoCyan
                ),
                maxLines = 4
            )
            
            // Send button
            FloatingActionButton(
                onClick = onSendClick,
                containerColor = KipepeoCyan,
                contentColor = KipepeoBlack,
                modifier = Modifier.size(56.dp)
            ) {
                Icon(
                    imageVector = Icons.Default.Send,
                    contentDescription = "Send"
                )
            }
        }
    }
}

@Composable
private fun VoiceButton(
    isRecording: Boolean,
    onClick: () -> Unit
) {
    val infiniteTransition = rememberInfiniteTransition(label = "voice")
    val scale by infiniteTransition.animateFloat(
        initialValue = 1f,
        targetValue = 1.15f,
        animationSpec = infiniteRepeatable(
            animation = tween(500),
            repeatMode = RepeatMode.Reverse
        ),
        label = "voice_scale"
    )
    
    FloatingActionButton(
        onClick = onClick,
        containerColor = if (isRecording) FireOrange else KipepeoGray,
        contentColor = TextPrimary,
        modifier = Modifier
            .size(56.dp)
            .then(if (isRecording) Modifier.scale(scale) else Modifier)
    ) {
        Icon(
            imageVector = Icons.Default.Mic,
            contentDescription = if (isRecording) "Stop recording" else "Start recording"
        )
    }
}

@Preview(showBackground = true, backgroundColor = 0xFF000000)
@Composable
private fun ChatScreenPreview() {
    KipepeoTheme {
        ChatScreen()
    }
}
