import SwiftUI

struct ChatMessage: Identifiable {
    let id = UUID()
    let text: String
    let isUser: Bool
}

struct ChatView: View {
    @State private var messages: [ChatMessage] = [
        ChatMessage(text: "Mambo! I am Kipepeo. How can I help you today?", isUser: false)
    ]
    @State private var inputText: String = ""
    
    var body: some View {
        VStack {
            // Chat History
            ScrollView {
                LazyVStack(alignment: .leading, spacing: 10) {
                    ForEach(messages) { message in
                        HStack {
                            if message.isUser { Spacer() }
                            
                            Text(message.text)
                                .padding()
                                .background(message.isUser ? Color("KipepeoCyan") : Color.gray.opacity(0.2))
                                .foregroundColor(message.isUser ? .black : .white)
                                .cornerRadius(15)
                            
                            if !message.isUser { Spacer() }
                        }
                    }
                }
                .padding()
            }
            
            // Input Area
            HStack {
                TextField("Type a message...", text: $inputText)
                    .padding(10)
                    .background(Color.gray.opacity(0.2))
                    .cornerRadius(20)
                    .foregroundColor(.white)
                
                Button(action: sendMessage) {
                    Image(systemName: "paperplane.fill")
                        .font(.system(size: 24))
                        .foregroundColor(Color("KipepeoCyan"))
                }
                
                Button(action: startVoice) {
                    Image(systemName: "mic.fill")
                        .font(.system(size: 24))
                        .foregroundColor(Color("KipepeoCyan"))
                }
            }
            .padding()
        }
        .background(Color.black.edgesIgnoringSafeArea(.all))
    }
    
    func sendMessage() {
        guard !inputText.isEmpty else { return }
        messages.append(ChatMessage(text: inputText, isUser: true))
        inputText = ""
        
        // Simulate AI response
        DispatchQueue.main.asyncAfter(deadline: .now() + 1) {
            messages.append(ChatMessage(text: "I am running on Metal! 🚀", isUser: false))
        }
    }
    
    func startVoice() {
        // Trigger voice input
    }
}
