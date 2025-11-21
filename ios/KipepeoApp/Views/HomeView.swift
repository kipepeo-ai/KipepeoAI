import SwiftUI
import KipepeoKit

struct HomeView: View {
    @State private var savedDataString: String = "0.0 MB"
    
    var body: some View {
        ZStack {
            Color.black.edgesIgnoringSafeArea(.all)
            
            VStack(spacing: 20) {
                // Header
                Text("Kipepeo")
                    .font(.largeTitle)
                    .fontWeight(.bold)
                    .foregroundColor(Color("KipepeoCyan"))
                
                Spacer()
                
                // Fire Animation Placeholder
                Circle()
                    .fill(
                        RadialGradient(gradient: Gradient(colors: [Color.orange, Color.red]), center: .center, startRadius: 5, endRadius: 100)
                    )
                    .frame(width: 200, height: 200)
                    .overlay(
                        Text("🔥")
                            .font(.system(size: 80))
                    )
                    .shadow(color: .orange, radius: 20, x: 0, y: 0)
                
                Text("Data saved today")
                    .font(.headline)
                    .foregroundColor(.gray)
                
                Text(savedDataString)
                    .font(.system(size: 40, weight: .bold, design: .rounded))
                    .foregroundColor(.white)
                
                Spacer()
                
                // Quick Actions
                HStack(spacing: 30) {
                    ActionIcon(icon: "safari", label: "Browser")
                    ActionIcon(icon: "phone.fill", label: "WhatsApp")
                    ActionIcon(icon: "play.rectangle.fill", label: "YouTube")
                }
                .padding(.bottom, 40)
            }
        }
        .onAppear {
            updateDataStats()
        }
    }
    
    func updateDataStats() {
        // In a real app, observe DataUsageTracker via Combine or Timer
        savedDataString = DataUsageTracker.shared.getFormattedSavedString()
    }
}

struct ActionIcon: View {
    let icon: String
    let label: String
    
    var body: some View {
        VStack {
            Image(systemName: icon)
                .font(.system(size: 30))
                .foregroundColor(.white)
                .frame(width: 60, height: 60)
                .background(Color.gray.opacity(0.3))
                .clipShape(Circle())
            
            Text(label)
                .font(.caption)
                .foregroundColor(.gray)
        }
    }
}
