import SwiftUI

struct ContentView: View {
    @AppStorage("hasOnboarded") private var hasOnboarded: Bool = false
    @State private var selectedTab = 0
    
    var body: some View {
        if !hasOnboarded {
            OnboardingView()
        } else {
            TabView(selection: $selectedTab) {

            HomeView()
                .tabItem {
                    Label("Home", systemImage: "house.fill")
                }
                .tag(0)
            
            ChatView()
                .tabItem {
                    Label("Chat", systemImage: "message.fill")
                }
                .tag(1)
            
            CallsView()
                .tabItem {
                    Label("Calls", systemImage: "phone.fill")
                }
                .tag(2)
            
            Text("Health")
                .tabItem {
                    Label("Health", systemImage: "heart.fill")
                }
                .tag(3)
                
            Text("Learn")
                .tabItem {
                    Label("Learn", systemImage: "book.fill")
                }
                .tag(4)
                
            Text("Money")
                .tabItem {
                    Label("Money", systemImage: "banknote.fill")
                }
                .tag(5)
        }
        .accentColor(Color("KipepeoCyan")) // Requires Asset Catalog color
    }
}
