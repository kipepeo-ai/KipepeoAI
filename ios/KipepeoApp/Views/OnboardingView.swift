import SwiftUI

struct OnboardingView: View {
    @AppStorage("hasOnboarded") private var hasOnboarded: Bool = false
    @State private var selectedLanguage: String = "English"
    
    let languages = ["English", "Kiswahili", "Sheng"]
    
    var body: some View {
        ZStack {
            Color.black.edgesIgnoringSafeArea(.all)
            
            VStack(spacing: 30) {
                Spacer()
                
                Image(systemName: "globe")
                    .font(.system(size: 80))
                    .foregroundColor(Color("KipepeoCyan"))
                
                Text("Karibu Kipepeo")
                    .font(.largeTitle)
                    .fontWeight(.bold)
                    .foregroundColor(.white)
                
                Text("Choose your language / Chagua lugha yako")
                    .font(.subheadline)
                    .foregroundColor(.gray)
                
                VStack(spacing: 15) {
                    ForEach(languages, id: \.self) { language in
                        Button(action: {
                            selectedLanguage = language
                        }) {
                            HStack {
                                Text(language)
                                    .fontWeight(.semibold)
                                Spacer()
                                if selectedLanguage == language {
                                    Image(systemName: "checkmark.circle.fill")
                                }
                            }
                            .padding()
                            .frame(maxWidth: .infinity)
                            .background(selectedLanguage == language ? Color("KipepeoCyan").opacity(0.2) : Color.gray.opacity(0.1))
                            .foregroundColor(selectedLanguage == language ? Color("KipepeoCyan") : .white)
                            .cornerRadius(12)
                            .overlay(
                                RoundedRectangle(cornerRadius: 12)
                                    .stroke(selectedLanguage == language ? Color("KipepeoCyan") : Color.clear, lineWidth: 2)
                            )
                        }
                    }
                }
                .padding(.horizontal, 30)
                
                Spacer()
                
                Button(action: {
                    hasOnboarded = true
                }) {
                    Text("Anza / Start")
                        .font(.headline)
                        .foregroundColor(.black)
                        .frame(maxWidth: .infinity)
                        .padding()
                        .background(Color("KipepeoCyan"))
                        .cornerRadius(30)
                }
                .padding(.horizontal, 30)
                .padding(.bottom, 50)
            }
        }
    }
}
