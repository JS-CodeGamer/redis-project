use std::num::{ParseFloatError, ParseIntError};

use crate::data_types::RedisProto;

#[derive(Debug, thiserror::Error)]
pub enum ProtoParseError {
    #[error("malformed input")]
    MalformedInput,
    #[error("incomplete input")]
    IncompleteInput,
    #[error("no implementation")]
    NoImpl,
    #[error("parse int error")]
    ParseIntError(#[from] ParseIntError),
    #[error("parse float error")]
    ParseFloatError(#[from] ParseFloatError),
}

impl From<i64> for RedisProto {
    fn from(value: i64) -> Self {
        Self::Int(value)
    }
}

impl From<f64> for RedisProto {
    fn from(value: f64) -> Self {
        Self::Double(value)
    }
}

impl From<bool> for RedisProto {
    fn from(value: bool) -> Self {
        Self::Bool(value)
    }
}

impl TryFrom<&str> for RedisProto {
    type Error = ProtoParseError;

    fn try_from(value: &str) -> Result<Self, ProtoParseError> {
        match value.chars().nth(0).unwrap() {
            '+' => RedisProto::parse_simple_string(value),
            //'-' => RedisProto::parse_simple_error(value),
            ':' => RedisProto::parse_int(value),
            '$' => RedisProto::parse_bulk_string(value),
            '*' => RedisProto::parse_array(value),
            '_' => Ok(Self::Null),
            '#' => value
                .chars()
                .nth(1)
                .ok_or(ProtoParseError::IncompleteInput)
                .map(|c| RedisProto::Bool(c == 't')),
            ',' => RedisProto::parse_double(value),
            //'!' => RedisProto::parse_bulk_error(value),
            _ => Err(ProtoParseError::NoImpl),
        }
    }
}

impl RedisProto {
    fn parse_simple_string(value: &str) -> Result<Self, ProtoParseError> {
        Ok(Self::SimpleString(value[1..].into()))
    }
    fn parse_int(value: &str) -> Result<Self, ProtoParseError> {
        value
            .lines()
            .next()
            .ok_or(ProtoParseError::IncompleteInput)?
            .chars()
            .skip(1)
            .collect::<String>()
            .parse::<i64>()
            .map(|x| x.into())
            .map_err(|x| x.into())
    }
    fn parse_bulk_string(value: &str) -> Result<Self, ProtoParseError> {
        let mut lines = value.lines();
        let len = lines.next().ok_or(ProtoParseError::IncompleteInput)?[1..].parse::<usize>()?;
        let value = lines.next().ok_or(ProtoParseError::IncompleteInput)?;
        if value.chars().count() != len {
            Err(ProtoParseError::IncompleteInput)
        } else {
            Ok(Self::BulkString(value.into()))
        }
    }
    fn parse_array(value: &str) -> Result<Self, ProtoParseError> {
        let mut lines = value.lines();
        let len = lines.next().ok_or(ProtoParseError::IncompleteInput)?[1..].parse::<usize>()?;
        let mut res = Vec::new();
        for _ in 1..=len {
            let mut value = lines
                .next()
                .ok_or(ProtoParseError::IncompleteInput)?
                .to_string();
            if value
                .chars()
                .nth(0)
                .ok_or(ProtoParseError::IncompleteInput)?
                == '$'
            {
                value.push_str("\r\n");
                value.push_str(lines.next().ok_or(ProtoParseError::IncompleteInput)?);
            }
            res.push(RedisProto::try_from(value.as_str())?);
        }
        Ok(Self::Array(res))
    }
    fn parse_double(value: &str) -> Result<Self, ProtoParseError> {
        value
            .lines()
            .next()
            .ok_or(ProtoParseError::IncompleteInput)?
            .chars()
            .skip(1)
            .collect::<String>()
            .parse::<f64>()
            .map(|x| x.into())
            .map_err(|x| x.into())
    }
}
