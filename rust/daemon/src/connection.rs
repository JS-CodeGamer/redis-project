use std::{collections::HashMap, io::Write};

use mio::net::TcpStream;
use protocol::RedisProto;

use crate::{commands::RedisCommand, error::RedisError};

pub struct Connection {
    pub stream: TcpStream,
    pub read_buf: Vec<u8>,
    pub bytes_read: usize,
}

impl Connection {
    pub fn new(stream: TcpStream) -> Self {
        Self {
            stream,
            read_buf: vec![0; 1024],
            bytes_read: 0,
        }
    }

    pub fn try_parse(&mut self, db: &mut HashMap<String, RedisProto>) {
        let extracted_string = String::from_utf8_lossy(&self.read_buf[..self.bytes_read]);
        let command_array = RedisProto::try_from(extracted_string.into_owned().as_str()).unwrap();
        let command = RedisCommand::parse(command_array).unwrap();
        let res = command.execute(db).unwrap();
        self.respond(command, res);
    }

    fn respond(&mut self, command: RedisCommand, res: Option<RedisProto>) {
        let resp = match command {
            RedisCommand::Ping => Ok(RedisProto::SimpleString("PONG".into())),
            RedisCommand::Get { key: _ } => res.ok_or(RedisError::NoKeyFound),
            RedisCommand::Set { key: _, value: _ } => Ok(RedisProto::SimpleString("OK".into())),
        };
        dbg!(&resp);
        let resp = resp
            .map(String::from)
            .map_err(|e| String::from(RedisProto::BulkErr(format!("{e}"))));
        dbg!(&resp);
        let _ = match resp {
            Ok(x) => self.stream.write_all(x.as_bytes()),
            Err(x) => self.stream.write_all(x.as_bytes()),
        };
    }
}
