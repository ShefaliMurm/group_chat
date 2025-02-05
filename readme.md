# README - Chat Server with Groups and Private Messages

## How to run the code

- Run make in the project directory in the terminal
- run the server in one terminal using ./server_grp
- start multiple clients on diiferent terminals using ./client_grp then use the group chat with the commands given

## Implemented Features:

- Multi-threaded TCP server
- User authentication
- Private messaging (/msg)
- Broadcasting messages to all users (/broadcast)
- Group creation (/create_group)
- Joining and leaving groups (/join_group, /leave_group)
- Sending messages to group members (/group_msg)

## Design Decisions

### Multi-threading vs Multi-processing:

- A new thread is created for each client connection to handle concurrent users efficiently.
- Multi-threading is preferred over multi-processing as it shares resources effectively and reduces memory overhead.

### Synchronization:

- `std::mutex` is used to protect shared resources like the `clients` and `groups` data structures.
- Ensuring only one thread modifies these data structures at a time prevents race conditions.

## Code Flow:

1. Server starts and listens on a port.
2. A new thread is created for each incoming client connection.
3. Client authentication occurs using `users.txt`.
4. Client sends commands which are parsed and executed.
5. Server synchronizes shared data and handles message passing.

## Testing

### Testing Strategies:

- **Correctness Testing**: Checked if private messages, group messages, and authentication work correctly.
- **Stress Testing**: Connected multiple clients simultaneously and verified concurrent messaging.
- **Edge Cases**: Tested invalid commands, duplicate usernames, and exceeding buffer sizes.

## Sources Referred

- GeeksforGeeks Networking Tutorials

## Declaration

I declare that this assignment was completed without any unauthorized assistance or plagiarism.

## Feedback

- The assignment was well-structured and helped in understanding socket programming deeply.
- Implementing synchronization for shared resources was challenging but insightful.
