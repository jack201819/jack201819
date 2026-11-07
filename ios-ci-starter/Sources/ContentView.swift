import SwiftUI

struct ContentView: View {
    var body: some View {
        VStack(spacing: 12) {
            Image(systemName: "swift")
                .font(.system(size: 48))
            Text(Greeting.text)
                .font(.headline)
        }
        .padding()
    }
}

enum Greeting {
    static let text = "Built on CI, running on your phone."
}

#Preview {
    ContentView()
}
