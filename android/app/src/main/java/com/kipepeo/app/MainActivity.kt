package com.kipepeo.app

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import com.kipepeo.app.ui.*
import com.kipepeo.app.ui.theme.KipepeoTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            KipepeoTheme {
                KipepeoApp()
            }
        }
    }
}

@Composable
fun KipepeoApp() {
    val navController = rememberNavController()
    var selectedLanguage by remember { mutableStateOf<Language?>(null) }
    var showReferralDialog by remember { mutableStateOf(false) }
    var showPaymentDialog by remember { mutableStateOf(false) }
    var showShareCard by remember { mutableStateOf(false) }
    
    Surface(
        modifier = Modifier.fillMaxSize(),
        color = MaterialTheme.colorScheme.background
    ) {
        Box {
            NavHost(
                navController = navController,
                startDestination = if (selectedLanguage == null) "onboarding" else "home"
            ) {
                composable("onboarding") {
                    OnboardingScreen(
                        onLanguageSelected = { language ->
                            selectedLanguage = language
                            navController.navigate("home") {
                                popUpTo("onboarding") { inclusive = true }
                            }
                        }
                    )
                }
                
                composable("home") {
                    HomeScreen(
                        onNavigateToChat = {
                            navController.navigate("chat")
                        },
                        onNavigateToCalls = {
                            navController.navigate("calls")
                        },
                        onNavigateToVision = {
                            navController.navigate("vision")
                        },
                        onNavigateToHealth = {
                            navController.navigate("health")
                        },
                        onNavigateToLearn = {
                            navController.navigate("learn")
                        },
                        onNavigateToMoney = {
                            navController.navigate("money")
                        },
                        onNavigateToZeroData = {
                            navController.navigate("zerodata")
                        },
                        onNavigateToMesh = {
                            navController.navigate("mesh")
                        },
                        onNavigateToViral = {
                            navController.navigate("viral")
                        },
                        onNavigateToTranslation = {
                            // TODO: Implement translation mode
                        },
                        onNavigateToKCSE = {
                            // TODO: Implement KCSE prep mode
                        }
                    )
                }
                
                composable("chat") {
                    ChatScreen(
                        onBack = {
                            navController.popBackStack()
                        }
                    )
                }

                composable("calls") {
                    com.kipepeo.app.calls.CallsScreen()
                }
                
                composable("vision") {
                    com.kipepeo.app.vision.VisionScreen(navController)
                }
                
                composable("vision_farmer") {
                    com.kipepeo.app.vision.FarmerModeScreen()
                }
                
                composable("vision_gen") {
                    com.kipepeo.app.vision.ImageGenScreen()
                }
                
                composable("vision_textbook") {
                    com.kipepeo.app.vision.TextbookModeScreen()
                }
                
                composable("health") {
                    com.kipepeo.app.health.HealthScreen()
                }
                
                composable("learn") {
                    com.kipepeo.app.learn.LearnScreen()
                }
                
                composable("money") {
                    com.kipepeo.app.money.MoneyScreen()
                }
                
                composable("zerodata") {
                    com.kipepeo.app.zerodata.ZeroDataScreen()
                }
                
                composable("mesh") {
                    com.kipepeo.app.mesh.MeshScreen()
                }
                
                composable("viral") {
                    com.kipepeo.app.viral.ViralScreen()
                }
            }
            
            // Viral Feature Overlays (can be triggered from settings/menu)
            if (showShareCard) {
                ShareCard(
                    dataSavedGB = 12.0f,
                    onShare = { platform ->
                        // TODO: Implement actual sharing logic
                        showShareCard = false
                    }
                )
            }
            
            if (showReferralDialog) {
                ReferralDialog(
                    referralCount = 2,
                    onDismiss = { showReferralDialog = false },
                    onInviteFriends = {
                        // TODO: Implement referral sharing
                        showReferralDialog = false
                    }
                )
            }
            
            if (showPaymentDialog) {
                MPesaPaymentDialog(
                    onDismiss = { showPaymentDialog = false },
                    onPayNow = { phoneNumber ->
                        // TODO: Implement M-Pesa STK push
                        showPaymentDialog = false
                    }
                )
            }
        }
    }
}
