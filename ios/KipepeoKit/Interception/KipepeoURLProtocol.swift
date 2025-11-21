import Foundation

/// KipepeoURLProtocol intercepts HTTP/HTTPS requests to save data on video streams.
/// It identifies video traffic and can redirect it through Kipepeo's compression engine.
@objc public class KipepeoURLProtocol: URLProtocol {
    
    private static let videoExtensions = ["mp4", "m3u8", "ts", "mov", "webm"]
    private static let videoMimeTypes = ["video/mp4", "application/vnd.apple.mpegurl", "video/quicktime", "video/webm"]
    
    public override class func canInit(with request: URLRequest) -> Bool {
        // Don't intercept if we've already handled it to avoid infinite loops
        if URLProtocol.property(forKey: "KipepeoHandled", in: request) != nil {
            return false
        }
        
        guard let url = request.url else { return false }
        
        // Check extension
        let ext = url.pathExtension.lowercased()
        if videoExtensions.contains(ext) {
            return true
        }
        
        // Check header (if available in request)
        // Note: This is often not available until response, but we check what we can.
        
        return false
    }
    
    public override class func canonicalRequest(for request: URLRequest) -> URLRequest {
        return request
    }
    
    public override func startLoading() {
        guard let newRequest = (request as NSURLRequest).mutableCopy() as? NSMutableURLRequest else { return }
        
        // Mark as handled
        URLProtocol.setProperty(true, forKey: "KipepeoHandled", in: newRequest)
        
        // Here is where the magic happens.
        // For Phase 2, we will just pass-through but count bytes.
        // In future, we would rewrite the URL to point to a local proxy (127.0.0.1:port)
        // that performs the AV1 transcoding on the fly.
        
        let session = URLSession(configuration: .default)
        let task = session.dataTask(with: newRequest as URLRequest) { [weak self] data, response, error in
            guard let self = self else { return }
            
            if let error = error {
                self.client?.urlProtocol(self, didFailWithError: error)
                return
            }
            
            if let response = response {
                self.client?.urlProtocol(self, didReceive: response, cacheStoragePolicy: .notAllowed)
                
                // Track Data Usage
                if let data = data {
                    // Notify DataUsageTracker (Stub)
                    // DataUsageTracker.shared.track(bytes: data.count)
                    self.client?.urlProtocol(self, didLoad: data)
                }
                
                self.client?.urlProtocolDidFinishLoading(self)
            }
        }
        task.resume()
    }
    
    public override func stopLoading() {
        // Cancel any running tasks
    }
}
