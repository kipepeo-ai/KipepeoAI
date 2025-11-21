import Foundation

@objc public class DataUsageTracker: NSObject {
    @objc public static let shared = DataUsageTracker()
    
    private var _totalBytesUsed: Int64 = 0
    private var _totalBytesSaved: Int64 = 0
    
    public var totalBytesUsed: Int64 {
        get { return _totalBytesUsed }
    }
    
    public var totalBytesSaved: Int64 {
        get { return _totalBytesSaved }
    }
    
    public func trackUsage(original: Int64, actual: Int64) {
        _totalBytesUsed += actual
        _totalBytesSaved += (original - actual)
    }
    
    public func getFormattedSavedString() -> String {
        let mb = Double(_totalBytesSaved) / 1024.0 / 1024.0
        if mb > 1024 {
            return String(format: "%.1f GB", mb / 1024.0)
        } else {
            return String(format: "%.1f MB", mb)
        }
    }
}
