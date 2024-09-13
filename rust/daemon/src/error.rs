pub type Result<T> = std::result::Result<T, RedisError>;

#[derive(Debug, thiserror::Error)]
pub enum RedisError {
    #[error("io error")]
    IOError(#[from] std::io::Error),
    #[error("key does not exist")]
    NoKeyFound,
}
