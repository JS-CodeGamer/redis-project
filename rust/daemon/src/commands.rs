use std::collections::HashMap;

use protocol::RedisProto;

#[derive(Debug)]
pub enum RedisCommand {
    Get { key: String },
    Set { key: String, value: RedisProto },
    Ping,
}

#[derive(Debug, thiserror::Error)]
pub enum CommandError {
    #[error("couldnt parse input into command")]
    ParseError,
    #[error("invalid length")]
    InvalidLength,
    #[error("invalid command")]
    InvalidCommand,
    #[error("invalid key")]
    InvalidKey,
}

impl RedisCommand {
    pub fn parse(command_array: RedisProto) -> Result<Self, CommandError> {
        let RedisProto::Array(command_array) = command_array else {
            return Err(CommandError::ParseError);
        };
        let mut command_array = command_array.iter();
        let RedisProto::BulkString(command_str) =
            command_array.next().ok_or(CommandError::InvalidLength)?
        else {
            return Err(CommandError::ParseError);
        };
        match command_str.to_lowercase().as_str() {
            "get" => {
                let RedisProto::BulkString(key) =
                    command_array.next().ok_or(CommandError::InvalidLength)?
                else {
                    return Err(CommandError::ParseError);
                };
                Ok(Self::Get { key: key.into() })
            }
            "set" => {
                let (RedisProto::BulkString(key), value) = (
                    command_array.next().ok_or(CommandError::InvalidLength)?,
                    command_array.next().ok_or(CommandError::InvalidLength)?,
                ) else {
                    return Err(CommandError::ParseError);
                };
                Ok(Self::Set {
                    key: key.into(),
                    value: value.clone(),
                })
            }
            "ping" => Ok(Self::Ping),
            _ => Err(CommandError::InvalidCommand),
        }
    }
    pub fn execute(
        &self,
        db: &mut HashMap<String, RedisProto>,
    ) -> Result<Option<RedisProto>, CommandError> {
        match self {
            Self::Get { key } => db
                .get(key)
                .ok_or(CommandError::InvalidKey)
                .map(|x| Some(x.clone())),
            Self::Set { key, value } => {
                db.insert(key.clone(), value.clone());
                Ok(None)
            }
            Self::Ping => Ok(None),
        }
    }
}
