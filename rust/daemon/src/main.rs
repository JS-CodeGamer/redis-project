use std::collections::HashMap;
use std::io::{self, Read};
use std::net::SocketAddr;

use mio::{net::TcpListener, Events, Interest, Poll, Token};

use crate::connection::Connection;

mod commands;
mod connection;
mod error;

const SERVER_IDENT: Token = Token(0);
const MAX_CONNECTIONS: usize = 1024;

fn main() -> io::Result<()> {
    let mut events = Events::with_capacity(1024);
    let mut poll = Poll::new()?;

    let addr: SocketAddr = "127.0.0.1:6379".parse().unwrap();
    let mut server = TcpListener::bind(addr)?;

    let mut connection_count = 1;
    let mut connections = HashMap::new();

    let mut db = HashMap::new();

    poll.registry()
        .register(&mut server, SERVER_IDENT, Interest::READABLE)?;

    loop {
        poll.poll(&mut events, None)?;

        events.iter().try_for_each(|e| {
            match e.token() {
                SERVER_IDENT => loop {
                    print!("server event: ");
                    match server.accept() {
                        Ok((mut stream, addr)) => {
                            println!("connection accepted from {:?}", addr);
                            if connections.keys().len() == MAX_CONNECTIONS {
                                return Ok::<(), io::Error>(());
                            }

                            let token = Token(connection_count);
                            connection_count += 1;

                            poll.registry()
                                .register(&mut stream, token, Interest::READABLE)?;

                            connections.insert(token, Connection::new(stream));
                        }
                        Err(ref e) if e.kind() == io::ErrorKind::WouldBlock => {
                            println!("would block err event");
                            break;
                        }
                        e => panic!("err={:?}", e),
                    }
                },
                client_ident => {
                    print!("connection event: ");
                    let connection = connections.get_mut(&client_ident).unwrap();
                    loop {
                        match connection.stream.read(&mut connection.read_buf) {
                            Ok(0) => {
                                println!("end connection");
                                connections.remove(&client_ident);
                            }
                            Ok(n) => {
                                println!("read {n} bytes");
                                connection.bytes_read += n;
                                if connection.bytes_read == connection.read_buf.capacity() {
                                    connection.read_buf.resize(connection.bytes_read + 1024, 0);
                                    continue;
                                }
                                connection.try_parse(&mut db);
                            }
                            Err(ref e) if e.kind() == io::ErrorKind::WouldBlock => {
                                println!("would block error");
                            }
                            e => panic!("err={:?}", e),
                        }
                        break;
                    }
                }
            };
            Ok(())
        })?
    }
}
