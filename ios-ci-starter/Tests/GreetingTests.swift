import XCTest
@testable import App

final class GreetingTests: XCTestCase {
    func testGreetingIsNotEmpty() {
        XCTAssertFalse(Greeting.text.isEmpty)
    }
}
