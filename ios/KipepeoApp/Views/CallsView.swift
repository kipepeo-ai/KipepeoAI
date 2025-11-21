import SwiftUI

struct CallsView: View {
    var body: some View {
        NavigationView {
            List {
                Section(header: Text("Recent")) {
                    CallRow(name: "Mama", time: "Today, 10:30 AM", icon: "phone.fill.arrow.up.right", color: .green)
                    CallRow(name: "Juma", time: "Yesterday, 8:15 PM", icon: "phone.fill.arrow.down.left", color: .red)
                }
                
                Section(header: Text("Nearby (Mesh)")) {
                    Text("Scanning for nearby devices...")
                        .italic()
                        .foregroundColor(.gray)
                }
            }
            .listStyle(InsetGroupedListStyle())
            .navigationTitle("Calls")
        }
    }
}

struct CallRow: View {
    let name: String
    let time: String
    let icon: String
    let color: Color
    
    var body: some View {
        HStack {
            Image(systemName: "person.circle.fill")
                .font(.system(size: 40))
                .foregroundColor(.gray)
            
            VStack(alignment: .leading) {
                Text(name)
                    .font(.headline)
                Text(time)
                    .font(.caption)
                    .foregroundColor(.gray)
            }
            
            Spacer()
            
            Image(systemName: icon)
                .foregroundColor(color)
        }
    }
}
