import Foundation

@objc public class KipepeoManager: NSObject {
    @objc public static let shared = KipepeoManager()
    
    @objc public func start() {
        print("[Kipepeo] Starting KipepeoKit...")
        
        // Register URL Protocol
        URLProtocol.registerClass(KipepeoURLProtocol.self)
        
        // Enable Swizzling
        AVPlayer.enableKipepeoSwizzling()
        
        print("[Kipepeo] Interception active.")
    }
}
