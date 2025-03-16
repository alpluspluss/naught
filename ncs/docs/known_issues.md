### `std::string` creates memory leak if run frequently

Consider using `fbstring` from `folly` or any other string objects instead of the standard library as they are not 
suited for performance.

##### TBA