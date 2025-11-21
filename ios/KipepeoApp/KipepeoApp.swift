import SwiftUI
import KipepeoKit

@main
struct KipepeoApp: App {
    init() {
        // Initialize KipepeoKit (Hooks, Metal backend, etc.)
        KipepeoManager.shared.start()
    }
    
    var body: some Scene {
        WindowGroup {
            ContentView()
        }
    }
}
