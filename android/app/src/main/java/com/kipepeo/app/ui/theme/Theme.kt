package com.kipepeo.app.ui.theme

import android.app.Activity
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.SideEffect
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.platform.LocalView
import androidx.core.view.WindowCompat

private val KipepeoDarkColorScheme = darkColorScheme(
    primary = KipepeoCyan,
    onPrimary = KipepeoBlack,
    primaryContainer = KipepeoCyanDark,
    onPrimaryContainer = TextPrimary,
    
    secondary = KipepeoCyanLight,
    onSecondary = KipepeoBlack,
    secondaryContainer = KipepeoGray,
    onSecondaryContainer = TextSecondary,
    
    tertiary = FireOrange,
    onTertiary = TextPrimary,
    
    background = KipepeoBlack,
    onBackground = TextPrimary,
    
    surface = KipepeoBlackSoft,
    onSurface = TextPrimary,
    surfaceVariant = KipepeoGray,
    onSurfaceVariant = TextSecondary,
    
    error = ErrorRed,
    onError = TextPrimary,
    
    outline = KipepeoGrayLight,
    outlineVariant = KipepeoGray
)

@Composable
fun KipepeoTheme(
    darkTheme: Boolean = true, // Always dark theme for Kipepeo
    content: @Composable () -> Unit
) {
    val colorScheme = KipepeoDarkColorScheme
    
    val view = LocalView.current
    if (!view.isInEditMode) {
        SideEffect {
            val window = (view.context as Activity).window
            window.statusBarColor = colorScheme.background.toArgb()
            window.navigationBarColor = colorScheme.background.toArgb()
            WindowCompat.getInsetsController(window, view).isAppearanceLightStatusBars = false
        }
    }

    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography,
        content = content
    )
}
