package ai.kipepeo.ui.theme

import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color

private val KipepeoGreen = Color(0xFF4CAF50)
private val KipepeoGreenDark = Color(0xFF2E7D32)
private val KipepeoOrange = Color(0xFFFF9800)
private val KipepeoPurple = Color(0xFF9C27B0)

private val DarkColorScheme = darkColorScheme(
    primary = KipepeoGreen,
    secondary = KipepeoOrange,
    tertiary = KipepeoPurple,
    background = Color(0xFF121212),
    surface = Color(0xFF1E1E1E),
    onPrimary = Color.White,
    onSecondary = Color.Black,
    onBackground = Color.White,
    onSurface = Color.White
)

private val LightColorScheme = lightColorScheme(
    primary = KipepeoGreen,
    secondary = KipepeoOrange,
    tertiary = KipepeoPurple,
    background = Color(0xFFFFFBFE),
    surface = Color(0xFFFFFBFE),
    onPrimary = Color.White,
    onSecondary = Color.White,
    onBackground = Color(0xFF1C1B1F),
    onSurface = Color(0xFF1C1B1F)
)

@Composable
fun KipepeoTheme(
    darkTheme: Boolean = true,
    content: @Composable () -> Unit
) {
    val colorScheme = if (darkTheme) DarkColorScheme else LightColorScheme
    
    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography,
        content = content
    )
}
