package ai.kipepeo

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.viewModels
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import ai.kipepeo.ui.MainScreen
import ai.kipepeo.ui.theme.KipepeoTheme

class MainActivity : ComponentActivity() {
    
    private val viewModel: KipepeoViewModel by viewModels()
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        setContent {
            KipepeoTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    val state by viewModel.state.collectAsState()
                    
                    MainScreen(
                        state = state,
                        onActivateEngine = { viewModel.activateEngine() },
                        onDeactivateEngine = { viewModel.deactivateEngine() },
                        onResetStats = { viewModel.resetStatistics() }
                    )
                }
            }
        }
    }
}
